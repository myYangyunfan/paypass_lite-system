#include "ledger_system.h"
#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
typedef SOCKET socket_t;
#define CLOSE_SOCKET closesocket
#define SOCKET_INVALID INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
typedef int socket_t;
#define CLOSE_SOCKET close
#define SOCKET_INVALID (-1)
#endif

static LedgerSystem g_sys;

static void url_decode(char *dst, const char *src, int dst_size) {
    int i = 0, j = 0;
    while (src[i] && j < dst_size - 1) {
        if (src[i] == '%' && isxdigit((unsigned char)src[i+1]) && isxdigit((unsigned char)src[i+2])) {
            char hex[3] = { src[i+1], src[i+2], 0 };
            dst[j++] = (char)strtol(hex, NULL, 16);
            i += 3;
        } else if (src[i] == '+') {
            dst[j++] = ' ';
            i++;
        } else {
            dst[j++] = src[i++];
        }
    }
    dst[j] = 0;
}

static int get_param(const char *body, const char *key, char *out, int out_size) {
    int key_len = (int)strlen(key);
    const char *p = body;
    while (p && *p) {
        if ((p == body || *(p - 1) == '&') && strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            const char *val_start = p + key_len + 1;
            const char *val_end = strchr(val_start, '&');
            int val_len = val_end ? (int)(val_end - val_start) : (int)strlen(val_start);
            if (val_len >= out_size) val_len = out_size - 1;
            
            char *temp = (char *)malloc(val_len + 1);
            if (!temp) return 0;
            memcpy(temp, val_start, val_len);
            temp[val_len] = '\0';
            
            url_decode(out, temp, out_size);
            free(temp);
            return 1;
        }
        p = strchr(p, '&');
        if (p) p++;
    }
    return 0;
}

static int get_double_param(const char *body, const char *key, double *out) {
    char val[64];
    if (!get_param(body, key, val, sizeof(val))) return 0;
    char *end;
    *out = strtod(val, &end);
    return end != val;
}

static int text_ok(char *buf, const char *message) {
    return sprintf(buf, "success:true\nmessage:%s\n", message);
}

static int text_err(char *buf, int status, const char *message) {
    return sprintf(buf, "success:false\nerror:%s\ncode:%d\n", message, status);
}

static int text_account(char *buf, const Account *a) {
    return sprintf(buf,
        "account_number:%s\nuser_name:%s\nphone_number:%s\nbalance:%.2f\n",
        a->account_number, a->user_name, a->phone_number, a->balance);
}

static int text_ledger(char *buf, int buf_size, const Account *a) {
    int pos = 0;
    pos += snprintf(buf + pos, buf_size - pos,
        "success:true\naccount_number:%s\nuser_name:%s\nbalance:%.2f\n---\n",
        a->account_number, a->user_name, a->balance);

    if (!dll_empty(&a->ledger)) {
        Transaction *arr[1000];
        int count = 0;
        Transaction *t = dll_last(&a->ledger);
        while (t && t->txn_id > 0 && count < 1000) {
            arr[count++] = t;
            t = t->prev;
        }
        for (int i = count - 1; i >= 0; i--) {
            const char *type_str = txn_type_str(arr[i]->type);
            pos += snprintf(buf + pos, buf_size - pos,
                "%d,%s,%.2f,%s,%s,%s\n",
                arr[i]->txn_id, type_str, arr[i]->amount,
                arr[i]->from_account[0] ? arr[i]->from_account : "",
                arr[i]->to_account[0] ? arr[i]->to_account : "",
                arr[i]->timestamp);
        }
    }
    return pos;
}

static const char *status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 409: return "Conflict";
        case 422: return "Unprocessable Entity";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

static const char *mime_type(const char *path) {
    if (strstr(path, ".html")) return "text/html;charset=utf-8";
    if (strstr(path, ".css")) return "text/css;charset=utf-8";
    if (strstr(path, ".js")) return "application/javascript;charset=utf-8";
    if (strstr(path, ".png")) return "image/png";
    return "text/plain;charset=utf-8";
}

static int serve_file(const char *path, char *resp, int resp_size) {
    char filepath[512];
    if (strcmp(path, "/") == 0)
        snprintf(filepath, sizeof(filepath), "./web/static/index.html");
    else
        snprintf(filepath, sizeof(filepath), "./web/static%s", path);

    FILE *f = fopen(filepath, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = (char *)malloc(fsize + 1);
    if (!content) { fclose(f); return 0; }
    fread(content, 1, fsize, f);
    content[fsize] = 0;
    fclose(f);

    const char *mime = mime_type(filepath);
    int pos = snprintf(resp, resp_size,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n",
        mime, fsize);

    if (pos + fsize < resp_size) {
        memcpy(resp + pos, content, fsize);
        pos += fsize;
    }

    free(content);
    return pos;
}

static void send_response(socket_t client, int status, const char *body) {
    char resp[65536];
    int pos = snprintf(resp, sizeof(resp),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/plain;charset=utf-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n%s",
        status, status_text(status), (int)strlen(body), body);
    send(client, resp, pos, 0);
}

static void send_options(socket_t client) {
    const char *resp =
        "HTTP/1.1 204 No Content\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";
    send(client, resp, strlen(resp), 0);
}

static void extract_id(const char *path, const char *prefix, char *out, int out_size) {
    const char *start = path + strlen(prefix);
    int i = 0;
    while (*start && *start != '/' && i < out_size - 1) {
        out[i++] = *start++;
    }
    out[i] = 0;
}

static void handle_create_account(const char *body, socket_t client) {
    char acc_num[64] = "", user_name[128] = "", phone[32] = "";
    if (!get_param(body, "account_number", acc_num, sizeof(acc_num)) ||
        !get_param(body, "user_name", user_name, sizeof(user_name))) {
        char buf[256];
        text_err(buf, 400, "Missing account_number or user_name");
        send_response(client, 400, buf);
        return;
    }
    get_param(body, "phone_number", phone, sizeof(phone));
    if (strlen(acc_num) == 0 || strlen(user_name) == 0) {
        char buf[256];
        text_err(buf, 400, "Fields cannot be empty");
        send_response(client, 400, buf);
        return;
    }
    if (!ls_create_account(&g_sys, acc_num, user_name, phone)) {
        char buf[256];
        text_err(buf, 409, "Account exists");
        send_response(client, 409, buf);
        return;
    }
    Account *a = ls_search_account(&g_sys, acc_num);
    char abuf[512];
    text_account(abuf, a);
    char buf[1024];
    snprintf(buf, sizeof(buf), "success:true\nmessage:Created\n%s", abuf);
    send_response(client, 201, buf);
}

static void handle_deposit(const char *id, const char *body, socket_t client) {
    double amount = 0;
    if (!get_double_param(body, "amount", &amount)) {
        char buf[256];
        text_err(buf, 400, "Missing amount");
        send_response(client, 400, buf);
        return;
    }
    if (amount <= 0) {
        char buf[256];
        text_err(buf, 422, "Positive amount required");
        send_response(client, 422, buf);
        return;
    }
    if (!ls_deposit(&g_sys, id, amount)) {
        char buf[256];
        text_err(buf, 404, "Account not found");
        send_response(client, 404, buf);
        return;
    }
    Account *a = ls_search_account(&g_sys, id);
    char abuf[512];
    text_account(abuf, a);
    char buf[1024];
    snprintf(buf, sizeof(buf), "success:true\nmessage:Deposit OK\n%s", abuf);
    send_response(client, 200, buf);
}

static void handle_withdraw(const char *id, const char *body, socket_t client) {
    double amount = 0;
    if (!get_double_param(body, "amount", &amount)) {
        char buf[256];
        text_err(buf, 400, "Missing amount");
        send_response(client, 400, buf);
        return;
    }
    if (amount <= 0) {
        char buf[256];
        text_err(buf, 422, "Positive amount required");
        send_response(client, 422, buf);
        return;
    }
    if (!ls_withdraw(&g_sys, id, amount)) {
        char buf[256];
        text_err(buf, 422, "Insufficient balance or account not found");
        send_response(client, 422, buf);
        return;
    }
    Account *a = ls_search_account(&g_sys, id);
    char abuf[512];
    text_account(abuf, a);
    char buf[1024];
    snprintf(buf, sizeof(buf), "success:true\nmessage:Withdraw OK\n%s", abuf);
    send_response(client, 200, buf);
}

static void handle_transfer(const char *body, socket_t client) {
    char from[64] = "", to[64] = "";
    double amount = 0;
    if (!get_param(body, "from", from, sizeof(from)) ||
        !get_param(body, "to", to, sizeof(to)) ||
        !get_double_param(body, "amount", &amount)) {
        char buf[256];
        text_err(buf, 400, "Missing from/to/amount");
        send_response(client, 400, buf);
        return;
    }
    if (amount <= 0) {
        char buf[256];
        text_err(buf, 422, "Positive amount required");
        send_response(client, 422, buf);
        return;
    }
    if (strcmp(from, to) == 0) {
        char buf[256];
        text_err(buf, 422, "Cannot self-transfer");
        send_response(client, 422, buf);
        return;
    }
    if (!ls_transfer(&g_sys, from, to, amount)) {
        char buf[256];
        text_err(buf, 422, "Transfer failed");
        send_response(client, 422, buf);
        return;
    }
    Account *af = ls_search_account(&g_sys, from);
    Account *at = ls_search_account(&g_sys, to);
    char buf[1536];
    snprintf(buf, sizeof(buf),
        "success:true\nmessage:Transfer OK\n"
        "from_account_number:%s\nfrom_user_name:%s\nfrom_phone_number:%s\nfrom_balance:%.2f\n"
        "to_account_number:%s\nto_user_name:%s\nto_phone_number:%s\nto_balance:%.2f\n",
        af->account_number, af->user_name, af->phone_number, af->balance,
        at->account_number, at->user_name, at->phone_number, at->balance);
    send_response(client, 200, buf);
}

static void handle_search(const char *id, socket_t client) {
    Account *a = ls_search_account(&g_sys, id);
    if (!a) {
        char buf[256];
        text_err(buf, 404, "Not found");
        send_response(client, 404, buf);
        return;
    }
    char abuf[512];
    text_account(abuf, a);
    char buf[1024];
    snprintf(buf, sizeof(buf), "success:true\nmessage:Found\n%s", abuf);
    send_response(client, 200, buf);
}

static void handle_ledger(const char *id, socket_t client) {
    Account *a = ls_search_account(&g_sys, id);
    if (!a) {
        char buf[256];
        text_err(buf, 404, "Not found");
        send_response(client, 404, buf);
        return;
    }
    char buf[65536];
    text_ledger(buf, sizeof(buf), a);
    send_response(client, 200, buf);
}

static void handle_undo(socket_t client) {
    if (!ls_undo(&g_sys)) {
        char buf[256];
        text_err(buf, 422, "Nothing to undo");
        send_response(client, 422, buf);
        return;
    }
    char buf[256];
    text_ok(buf, "Undone");
    send_response(client, 200, buf);
}

static void handle_delete(const char *id, socket_t client) {
    if (!ls_delete_account(&g_sys, id)) {
        char buf[256];
        text_err(buf, 404, "Not found");
        send_response(client, 404, buf);
        return;
    }
    char buf[256];
    text_ok(buf, "Deleted");
    send_response(client, 200, buf);
}

static void route_request(char *method, char *path, char *body, socket_t client) {
    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/accounts") == 0) {
        handle_create_account(body, client);
    }
    else if (strcmp(method, "POST") == 0 && strstr(path, "/api/accounts/") == path) {
        char id[64];
        extract_id(path, "/api/accounts/", id, sizeof(id));
        if (strstr(path, "/deposit")) {
            handle_deposit(id, body, client);
        } else if (strstr(path, "/withdraw")) {
            handle_withdraw(id, body, client);
        } else {
            char buf[256];
            text_err(buf, 404, "Not Found");
            send_response(client, 404, buf);
        }
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/transfer") == 0) {
        handle_transfer(body, client);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/undo") == 0) {
        handle_undo(client);
    }
    else if (strcmp(method, "GET") == 0 && strstr(path, "/api/accounts/") == path) {
        char id[64];
        extract_id(path, "/api/accounts/", id, sizeof(id));
        if (strstr(path, "/ledger")) {
            handle_ledger(id, client);
        } else {
            int id_len = strlen(id);
            int prefix_len = strlen("/api/accounts/") + id_len;
            if ((int)strlen(path) == prefix_len) {
                handle_search(id, client);
            } else {
                char buf[256];
                text_err(buf, 404, "Not Found");
                send_response(client, 404, buf);
            }
        }
    }
    else if (strcmp(method, "DELETE") == 0 && strstr(path, "/api/accounts/") == path) {
        char id[64];
        extract_id(path, "/api/accounts/", id, sizeof(id));
        handle_delete(id, client);
    }
    else if (strcmp(method, "GET") == 0) {
        char file_resp[65536];
        int len = serve_file(path, file_resp, sizeof(file_resp));
        if (len > 0) {
            send(client, file_resp, len, 0);
        } else {
            char buf[256];
            text_err(buf, 404, "Not Found");
            send_response(client, 404, buf);
        }
    }
    else {
        char buf[256];
        text_err(buf, 404, "Not Found");
        send_response(client, 404, buf);
    }
}

static void handle_client(socket_t client) {
    char buf[16384];
    int total = 0;
    int header_end_pos = -1;

    while (total < (int)sizeof(buf) - 1) {
        int n = recv(client, buf + total, (int)sizeof(buf) - 1 - total, 0);
        if (n <= 0) break;
        total += n;
        buf[total] = 0;
        char *he = strstr(buf, "\r\n\r\n");
        if (he) {
            header_end_pos = (int)(he - buf) + 4;
            break;
        }
    }

    if (total <= 0) {
        CLOSE_SOCKET(client);
        return;
    }

    int content_length = 0;
    char *cl = strstr(buf, "Content-Length:");
    if (!cl) cl = strstr(buf, "content-length:");
    if (cl) {
        while (*cl && *cl != ':') cl++;
        cl++;
        while (*cl == ' ') cl++;
        content_length = atoi(cl);
    }

    if (header_end_pos > 0 && content_length > 0) {
        int body_received = total - header_end_pos;
        while (body_received < content_length && total < (int)sizeof(buf) - 1) {
            int n = recv(client, buf + total, (int)sizeof(buf) - 1 - total, 0);
            if (n <= 0) break;
            total += n;
            body_received += n;
        }
        buf[total] = 0;
    }

    char method[16] = "", path[512] = "";
    char *body = NULL;

    sscanf(buf, "%15s %511s", method, path);

    char decoded_path[512];
    url_decode(decoded_path, path, sizeof(decoded_path));

    if (header_end_pos > 0) {
        body = buf + header_end_pos;
    }

    if (strcmp(method, "OPTIONS") == 0) {
        send_options(client);
    } else {
        route_request(method, decoded_path, body ? body : "", client);
    }

    CLOSE_SOCKET(client);
}

int main() {
    ls_init(&g_sys);

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[FATAL] WSAStartup failed\n");
        return 1;
    }
#endif

    socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == SOCKET_INVALID) {
        fprintf(stderr, "[FATAL] socket() failed\n");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "[FATAL] bind() failed on port 8080 (in use?)\n");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        fprintf(stderr, "[FATAL] listen() failed\n");
        return 1;
    }

    printf("[SERVER] Listening on http://localhost:8080\n");
    fflush(stdout);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        socket_t client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == SOCKET_INVALID) continue;
        handle_client(client_fd);
    }

    ls_destroy(&g_sys);
    return 0;
}
