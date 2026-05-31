// ============================================================
// 支付系统轻量版 (PayPass Lite) - HTTP API 服务器
// 功能: 提供RESTful API接口，支持账户管理、转账交易等操作
// 平台: Windows (Winsock2) / Linux (POSIX sockets)
// ============================================================

#include "ledger_system.h"
#include "account.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <limits>

// Windows平台网络库初始化
#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef int socklen_t;
#else
  // Linux/Unix平台网络库
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #include <arpa/inet.h>
#endif

#include <fstream>

enum class JsonType { NUL, BOOL, NUMBER, STRING, ARRAY, OBJECT };

// JSON解析器类 - 支持JSON序列化与反序列化
// 用于HTTP请求体的解析和响应体的构建
class Json {
public:
    JsonType type;
    std::string str_val; double num_val; bool bool_val;
    std::vector<Json> arr_val;
    std::map<std::string, Json> obj_val;
    Json() : type(JsonType::NUL), num_val(0), bool_val(false) {}
    Json(double v) : type(JsonType::NUMBER), num_val(v), bool_val(false) {}
    Json(const std::string& v) : type(JsonType::STRING), str_val(v), num_val(0), bool_val(false) {}
    Json(const char* v) : type(JsonType::STRING), str_val(v), num_val(0), bool_val(false) {}
    Json(bool v) : type(JsonType::BOOL), num_val(0), bool_val(v) {}
    Json& operator[](const std::string& key) {
        if (type != JsonType::OBJECT) { type = JsonType::OBJECT; obj_val.clear(); }
        return obj_val[key];
    }
    const Json& operator[](const std::string& key) const {
        static Json n; auto it = obj_val.find(key);
        return it != obj_val.end() ? it->second : n;
    }
    bool contains(const std::string& key) const {
        return type == JsonType::OBJECT && obj_val.count(key);
    }
    template<typename T> T get() const;
    bool is_null() const { return type == JsonType::NUL; }
    std::string dump() const;
    static Json parse(const std::string& in, size_t& p);
    static Json parse(const std::string& in) { size_t p=0; return parse(in,p); }
private:
    static std::string esc(const std::string& s) {
        std::string o; for(char c:s){switch(c){
        case '"':o+="\\\"";break;case '\\':o+="\\\\";break;
        case '\b':o+="\\b";break;case '\f':o+="\\f";break;
        case '\n':o+="\\n";break;case '\r':o+="\\r";break;
        case '\t':o+="\\t";break;default:o+=c;}} return o;
    }
    static std::string unesc(const std::string& s) {
        std::string o; for(size_t i=0;i<s.size();++i){
        if(s[i]=='\\'&&i+1<s.size()){switch(s[i+1]){
        case'"':o+='"';++i;break;case'\\':o+='\\';++i;break;
        case'b':o+='\b';++i;break;case'f':o+='\f';++i;break;
        case'n':o+='\n';++i;break;case'r':o+='\r';++i;break;
        case't':o+='\t';++i;break;default:o+=s[i];}}else o+=s[i];} return o;
    }
    static void skipWs(const std::string& in, size_t& p) {
        while(p<in.size()&&(in[p]==' '||in[p]=='\t'||in[p]=='\n'||in[p]=='\r'))++p;
    }
    static Json parseObj(const std::string& in, size_t& p);
    static Json parseArr(const std::string& in, size_t& p);
    static Json parseStr(const std::string& in, size_t& p);
    static Json parseNum(const std::string& in, size_t& p);
};

template<> double Json::get<double>() const { return num_val; }
template<> int Json::get<int>() const { return (int)num_val; }
template<> std::string Json::get<std::string>() const { return str_val; }
template<> bool Json::get<bool>() const { return bool_val; }

std::string Json::dump() const {
    switch(type){
    case JsonType::NUL: return "null";
    case JsonType::BOOL: return bool_val?"true":"false";
    case JsonType::NUMBER: { std::ostringstream o; o<<num_val; return o.str(); }
    case JsonType::STRING: return '"'+esc(str_val)+'"';
    case JsonType::ARRAY: { std::string s="["; for(size_t i=0;i<arr_val.size();++i){if(i)s+=",";s+=arr_val[i].dump();} return s+"]"; }
    case JsonType::OBJECT: { std::string s="{"; bool f=true; for(auto&kv:obj_val){if(!f)s+=",";s+='"'+esc(kv.first)+"\":"+kv.second.dump();f=false;} return s+"}"; }
    } return "null";
}

Json Json::parse(const std::string& in, size_t& p) {
    skipWs(in,p); if(p>=in.size())return Json();
    char c=in[p];
    if(c=='{')return parseObj(in,p);
    if(c=='[')return parseArr(in,p);
    if(c=='"')return parseStr(in,p);
    if(c=='t'||c=='f'){Json b;b.type=JsonType::BOOL;b.bool_val=(c=='t');p+=(c=='t'?4:5);return b;}
    if(c=='n'){p+=4;return Json();}
    return parseNum(in,p);
}
Json Json::parseObj(const std::string& in, size_t& p){
    Json o;o.type=JsonType::OBJECT;++p;skipWs(in,p);
    if(p<in.size()&&in[p]=='}'){++p;return o;}
    while(true){skipWs(in,p);Json k=parseStr(in,p);skipWs(in,p);
    if(p>=in.size()||in[p]!=':')break;++p;o.obj_val[k.str_val]=parse(in,p);skipWs(in,p);
    if(p<in.size()&&in[p]==','){++p;continue;}break;}
    if(p<in.size()&&in[p]=='}')++p;return o;
}
Json Json::parseArr(const std::string& in, size_t& p){
    Json a;a.type=JsonType::ARRAY;++p;skipWs(in,p);
    if(p<in.size()&&in[p]==']'){++p;return a;}
    while(true){a.arr_val.push_back(parse(in,p));skipWs(in,p);
    if(p<in.size()&&in[p]==','){++p;continue;}break;}
    if(p<in.size()&&in[p]==']')++p;return a;
}
Json Json::parseStr(const std::string& in, size_t& p){
    Json s;s.type=JsonType::STRING;++p;std::string raw;
    while(p<in.size()&&in[p]!='"'){if(in[p]=='\\'&&p+1<in.size()){raw+=in[p];raw+=in[p+1];p+=2;}else raw+=in[p++];}
    if(p<in.size())++p;s.str_val=unesc(raw);return s;
}
Json Json::parseNum(const std::string& in, size_t& p){
    Json n;n.type=JsonType::NUMBER;size_t s=p;
    if(p<in.size()&&(in[p]=='-'||in[p]=='+'))++p;
    while(p<in.size()&&isdigit(in[p]))++p;
    if(p<in.size()&&in[p]=='.'){++p;while(p<in.size()&&isdigit(in[p]))++p;}
    n.num_val=stod(in.substr(s,p-s));return n;
}

// HTTP server
// HTTP请求结构体 - 存储解析后的请求信息
struct HttpRequest {
    std::string method, path, body;
    std::map<std::string,std::string> headers, params;
};

// HTTP响应结构体 - 用于构建响应内容
struct HttpResponse {
    int status=200; std::string body;
    std::map<std::string,std::string> headers;
    void set_content(const std::string& c, const std::string& t){body=c;headers["Content-Type"]=t;}
    void set_header(const std::string& k, const std::string& v){headers[k]=v;}
};

// 路由处理函数类型别名
using Handler = std::function<void(const HttpRequest&, HttpResponse&)>;

// 路由结构体 - 关联HTTP方法、路径前缀和处理函数
struct Route { std::string method, prefix; Handler handler; };

// 简易HTTP服务器类 - 封装socket监听和请求处理
class SimpleHttpServer {
    #ifdef _WIN32
    SOCKET fd_;
    #else
    int fd_;
    #endif
    int port_; std::vector<Route> routes_; std::string static_dir_;
    static std::string urldec(const std::string& s){
        std::string o; for(size_t i=0;i<s.size();++i){
        if(s[i]=='%'&&i+2<s.size()){o+=(char)strtol(s.substr(i+1,2).c_str(),0,16);i+=2;}
        else if(s[i]=='+')o+=' ';else o+=s[i];} return o;
    }
    static std::vector<std::string> split(const std::string& s, char d){
        std::vector<std::string> r; std::istringstream is(s); std::string t;
        while(getline(is,t,d)) if(!t.empty()) r.push_back(t); return r;
    }
    static const char* stxt(int c){switch(c){case 200:return"OK";case 201:return"Created";
    case 204:return"No Content";case 400:return"Bad Request";case 404:return"Not Found";
    case 409:return"Conflict";case 422:return"Unprocessable Entity";case 500:return"Internal Server Error";
    default:return"Unknown";}}
    HttpRequest parseReq(const std::string& raw){
        HttpRequest r; size_t he=raw.find("\r\n\r\n");
        std::string hs=(he!=std::string::npos)?raw.substr(0,he):raw;
        size_t nl=hs.find("\r\n"); std::string rl=(nl!=std::string::npos)?hs.substr(0,nl):hs;
        auto p=split(rl,' '); if(p.size()>=2){r.method=p[0];std::string fp=urldec(p[1]);
        size_t q=fp.find('?');if(q!=std::string::npos){r.path=fp.substr(0,q);
        std::string qs=fp.substr(q+1);size_t pos=0;while(pos<qs.size()){size_t eq=qs.find('=',pos);
        size_t am=qs.find('&',pos);if(eq!=std::string::npos&&eq<am){r.params[qs.substr(pos,eq-pos)]=
        qs.substr(eq+1,(am!=std::string::npos?am:qs.size())-eq-1);}if(am==std::string::npos)break;pos=am+1;}}
        else r.path=fp;}
        if(nl!=std::string::npos){size_t pos=nl+2;while(pos<hs.size()){size_t n=hs.find("\r\n",pos);
        std::string l=hs.substr(pos,n-pos);size_t c=l.find(':');if(c!=std::string::npos){
        std::string k=l.substr(0,c),v=l.substr(c+1);size_t s=v.find_first_not_of(" \t");
        if(s!=std::string::npos)v=v.substr(s);r.headers[k]=v;}if(n==std::string::npos)break;pos=n+2;}}
        if(he!=std::string::npos)r.body=raw.substr(he+4);return r;
    }
    std::string buildResp(const HttpResponse& r){
        std::ostringstream o; o<<"HTTP/1.1 "<<r.status<<" "<<stxt(r.status)<<"\r\n";
        for(auto&h:r.headers)o<<h.first<<": "<<h.second<<"\r\n";
        o<<"Content-Length: "<<r.body.size()<<"\r\nConnection: close\r\n\r\n"<<r.body; return o.str();
    }
    void addCors(HttpResponse& r){
        r.set_header("Access-Control-Allow-Origin","*");
        r.set_header("Access-Control-Allow-Methods","GET, POST, DELETE, OPTIONS");
        r.set_header("Access-Control-Allow-Headers","Content-Type");
    }
    static std::string mimeType(const std::string& p){
        if(p.find(".html")!=std::string::npos)return"text/html;charset=utf-8";
        if(p.find(".css")!=std::string::npos)return"text/css;charset=utf-8";
        if(p.find(".js")!=std::string::npos)return"application/javascript;charset=utf-8";
        if(p.find(".json")!=std::string::npos)return"application/json";
        if(p.find(".png")!=std::string::npos)return"image/png";
        return"text/plain;charset=utf-8";
    }
    bool serveFile(const std::string& path, HttpResponse& resp){
        if(static_dir_.empty())return false;
        std::string fp=path=="/"?"/index.html":path;
        std::string full=static_dir_+fp;
        std::ifstream f(full,std::ios::binary|std::ios::ate);
        if(!f)return false;
        std::streamsize sz=f.tellg();f.seekg(0);
        std::string content(static_cast<size_t>(sz),'\0');f.read(&content[0],sz);
        resp.status=200;resp.set_content(content,mimeType(fp));return true;
    }
    void handleClient(
        #ifdef _WIN32
        SOCKET c
        #else
        int c
        #endif
    ){
        char b[16384];int n=recv(c,b,sizeof(b)-1,0);
        if(n<=0){
            #ifdef _WIN32
            closesocket(c);
            #else
            close(c);
            #endif
            return;
        }
        b[n]=0;std::string raw(b);HttpRequest req=parseReq(raw);HttpResponse resp;
        if(req.method=="OPTIONS"){resp.status=204;addCors(resp);
        std::string r=buildResp(resp);send(c,r.c_str(),r.size(),0);
        #ifdef _WIN32
        closesocket(c);
        #else
        close(c);
        #endif
        return;}
        bool matched=false;
        if(req.method=="GET"&&serveFile(req.path,resp)){matched=true;}
        else{for(auto&rt:routes_){if(req.method!=rt.method)continue;
        bool exact=(rt.prefix.back()!='/');
        if((exact&&req.path==rt.prefix)||(!exact&&req.path.compare(0,rt.prefix.size(),rt.prefix)==0)){rt.handler(req,resp);matched=true;break;}}}
        if(!matched){resp.status=404;Json e;e["error"]="Not Found";
        e["message"]="No route for "+req.method+" "+req.path;resp.set_content(e.dump(),"application/json");}
        addCors(resp);std::string r=buildResp(resp);send(c,r.c_str(),r.size(),0);
        #ifdef _WIN32
        closesocket(c);
        #else
        close(c);
        #endif
    }
public:
    SimpleHttpServer(int p):
        #ifdef _WIN32
        fd_(INVALID_SOCKET),
        #else
        fd_(-1),
        #endif
        port_(p){}
    ~SimpleHttpServer(){
        #ifdef _WIN32
        if(fd_!=INVALID_SOCKET)closesocket(fd_);
        WSACleanup();
        #else
        if(fd_>=0)close(fd_);
        #endif
    }
    void get(const std::string& p, Handler h){routes_.push_back({"GET",p,h});}
    void post(const std::string& p, Handler h){routes_.push_back({"POST",p,h});}
    void del(const std::string& p, Handler h){routes_.push_back({"DELETE",p,h});}
    void setStaticDir(const std::string& d){static_dir_=d;}
    bool start(){
        #ifdef _WIN32
        WSADATA wsa;
        if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
            std::cerr<<"[FATAL] WSAStartup failed\n";return false;
        }
        #endif
        fd_=socket(AF_INET,SOCK_STREAM,0);
        #ifdef _WIN32
        if(fd_==INVALID_SOCKET){
            std::cerr<<"[FATAL] socket() failed\n";return false;
        }
        #else
        if(fd_<0){std::cerr<<"[FATAL] socket() failed\n";return false;}
        #endif
        int opt=1;setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,(const char*)&opt,sizeof(opt));
        struct sockaddr_in a;a.sin_family=AF_INET;a.sin_addr.s_addr=INADDR_ANY;a.sin_port=htons(port_);
        if(bind(fd_,(struct sockaddr*)&a,sizeof(a))<0){
            std::cerr<<"[FATAL] bind() failed on port "<<port_<<" (in use?)\n";return false;
        }
        if(listen(fd_,10)<0){
            std::cerr<<"[FATAL] listen() failed\n";return false;
        }
        std::cout<<"[SERVER] Listening on http://localhost:"<<port_<<"\n"<<std::flush;
        while(true){struct sockaddr_in ca;socklen_t cl=sizeof(ca);
        #ifdef _WIN32
        SOCKET cf=accept(fd_,(struct sockaddr*)&ca,&cl);
        if(cf==INVALID_SOCKET)continue;
        #else
        int cf=accept(fd_,(struct sockaddr*)&ca,&cl);
        if(cf<0)continue;
        #endif
        handleClient(cf);}
        return true;
    }
};

// 从URL路径中提取账户ID的辅助函数
std::string extractId(const std::string& path, const std::string& prefix){
    std::string r=path.substr(prefix.size());size_t s=r.find('/');
    return (s!=std::string::npos)?r.substr(0,s):r;
}

// 错误响应JSON构建函数
Json errJson(int s, const std::string& m){Json j;j["success"]=false;j["error"]=m;j["code"]=(double)s;return j;}

// 成功响应JSON构建函数
Json okJson(const std::string& m){Json j;j["success"]=true;j["message"]=m;return j;}

// 账户信息转JSON函数
Json accJson(const Account& a){Json j;j["account_number"]=a.getAccountNumber();
j["user_name"]=a.getUserName();j["phone_number"]=a.getPhoneNumber();j["balance"]=a.getBalance();return j;}

// 账本信息转JSON函数 (包含完整交易历史)
Json ledgerJson(const Account& a){
    Json l;l["account_number"]=a.getAccountNumber();l["user_name"]=a.getUserName();l["balance"]=a.getBalance();
    Json txs;txs.type=JsonType::ARRAY;const Transaction* t=a.getLastTransaction();
    std::vector<const Transaction*> v;while(t&&t->txn_id>0){v.push_back(t);t=t->prev;}
    for(auto it=v.rbegin();it!=v.rend();++it){Json j;j["txn_id"]=(double)(*it)->txn_id;
    j["type"]=txnTypeToStr((*it)->type);j["amount"]=(*it)->amount;
    j["from"]=(*it)->from_account.empty()?Json():Json((*it)->from_account);
    j["to"]=(*it)->to_account.empty()?Json():Json((*it)->to_account);
    j["timestamp"]=(*it)->timestamp;txs.arr_val.push_back(j);}
    l["transactions"]=txs;l["transaction_count"]=(double)txs.arr_val.size();return l;
}

// 程序入口 - 初始化账本系统并启动HTTP服务器
int main(){
    LedgerSystem sys;SimpleHttpServer srv(8080);
    srv.setStaticDir("./web/static");

    // 创建账户接口: POST /api/accounts
    srv.post("/api/accounts",[&](const HttpRequest& req,HttpResponse& resp){
        try{Json b=Json::parse(req.body);
        if(!b.contains("account_number")||!b.contains("user_name")){resp.status=400;
        resp.set_content(errJson(400,"Missing account_number or user_name").dump(),"application/json");return;}
        std::string n=b["account_number"].get<std::string>(),u=b["user_name"].get<std::string>();
        std::string p=b.contains("phone_number")?b["phone_number"].get<std::string>():"";
        if(n.empty()||u.empty()){resp.status=400;
        resp.set_content(errJson(400,"Fields cannot be empty").dump(),"application/json");return;}
        if(!sys.createAccount(n,u,p)){resp.status=409;
        resp.set_content(errJson(409,"Account exists").dump(),"application/json");return;}
        Account* a=sys.searchAccount(n);resp.status=201;
        Json r=okJson("Created");r["account"]=accJson(*a);resp.set_content(r.dump(),"application/json");
        }catch(...){resp.status=400;resp.set_content(errJson(400,"Invalid JSON").dump(),"application/json");}
    });

    // 存款/取款接口: POST /api/accounts/{id}/deposit 或 /withdraw
    srv.post("/api/accounts/",[&](const HttpRequest& req,HttpResponse& resp){
        std::string id=extractId(req.path,"/api/accounts/");
        if(req.path.find("/deposit")!=std::string::npos){
            try{Json b=Json::parse(req.body);
            if(!b.contains("amount")){resp.status=400;resp.set_content(errJson(400,"Missing amount").dump(),"application/json");return;}
            double a=b["amount"].get<double>();if(a<=0){resp.status=422;resp.set_content(errJson(422,"Positive amount required").dump(),"application/json");return;}
            if(!sys.deposit(id,a)){resp.status=404;resp.set_content(errJson(404,"Account not found").dump(),"application/json");return;}
            Json r=okJson("Deposit OK");
            Account* ac=sys.searchAccount(id);r["account"]=accJson(*ac);
            resp.set_content(r.dump(),"application/json");
            }catch(...){resp.status=400;resp.set_content(errJson(400,"Invalid JSON").dump(),"application/json");}
        }else if(req.path.find("/withdraw")!=std::string::npos){
            try{Json b=Json::parse(req.body);
            if(!b.contains("amount")){resp.status=400;resp.set_content(errJson(400,"Missing amount").dump(),"application/json");return;}
            double a=b["amount"].get<double>();if(a<=0){resp.status=422;resp.set_content(errJson(422,"Positive amount required").dump(),"application/json");return;}
            Account* ac=sys.searchAccount(id);if(!ac){resp.status=404;resp.set_content(errJson(404,"Account not found").dump(),"application/json");return;}
            if(!ac->canWithdraw(a)){resp.status=422;resp.set_content(errJson(422,"Insufficient balance").dump(),"application/json");return;}
            sys.withdraw(id,a);
            Json r=okJson("Withdraw OK");r["account"]=accJson(*ac);
            resp.set_content(r.dump(),"application/json");
            }catch(...){resp.status=400;resp.set_content(errJson(400,"Invalid JSON").dump(),"application/json");}
        }
    });

    // 转账接口: POST /api/transfer
    srv.post("/api/transfer",[&](const HttpRequest& req,HttpResponse& resp){
        try{Json b=Json::parse(req.body);
        if(!b.contains("from")||!b.contains("to")||!b.contains("amount")){resp.status=400;
        resp.set_content(errJson(400,"Missing from/to/amount").dump(),"application/json");return;}
        std::string f=b["from"].get<std::string>(),t=b["to"].get<std::string>();
        double a=b["amount"].get<double>();if(a<=0){resp.status=422;resp.set_content(errJson(422,"Positive amount required").dump(),"application/json");return;}
        if(f==t){resp.status=422;resp.set_content(errJson(422,"Cannot self-transfer").dump(),"application/json");return;}
        if(!sys.transfer(f,t,a)){resp.status=422;resp.set_content(errJson(422,"Transfer failed").dump(),"application/json");return;}
        Json r=okJson("Transfer OK");Account*af=sys.searchAccount(f),*at=sys.searchAccount(t);
        r["from_account"]=accJson(*af);r["to_account"]=accJson(*at);resp.set_content(r.dump(),"application/json");
        }catch(...){resp.status=400;resp.set_content(errJson(400,"Invalid JSON").dump(),"application/json");}
    });

    // 查询账户/账本接口: GET /api/accounts/{id} 或 /api/accounts/{id}/ledger
    srv.get("/api/accounts/",[&](const HttpRequest& req,HttpResponse& resp){
        std::string id=extractId(req.path,"/api/accounts/");
        size_t sl=std::string("/api/accounts/").size()+id.size();
        if(req.path.size()==sl){
            Account* a=sys.searchAccount(id);if(!a){resp.status=404;resp.set_content(errJson(404,"Not found").dump(),"application/json");return;}
            Json r=okJson("Found");r["account"]=accJson(*a);resp.set_content(r.dump(),"application/json");
        }else if(req.path.find("/ledger")!=std::string::npos){
            Account* a=sys.searchAccount(id);if(!a){resp.status=404;resp.set_content(errJson(404,"Not found").dump(),"application/json");return;}
            resp.set_content(ledgerJson(*a).dump(),"application/json");
        }
    });

    // 撤销操作接口: POST /api/undo
    srv.post("/api/undo",[&](const HttpRequest&,HttpResponse& resp){
        if(!sys.undo()){resp.status=422;resp.set_content(errJson(422,"Nothing to undo").dump(),"application/json");return;}
        resp.set_content(okJson("Undone").dump(),"application/json");
    });

    // 删除账户接口: DELETE /api/accounts/{id}
    srv.del("/api/accounts/",[&](const HttpRequest& req,HttpResponse& resp){
        std::string id=extractId(req.path,"/api/accounts/");
        size_t sl=std::string("/api/accounts/").size()+id.size();
        if(req.path.size()!=sl)return;
        if(!sys.deleteAccount(id)){resp.status=404;resp.set_content(errJson(404,"Not found").dump(),"application/json");return;}
        resp.set_content(okJson("Deleted").dump(),"application/json");
    });

    if(!srv.start()){std::cerr<<"[FATAL] Failed to start\n";return 1;}
    return 0;
}
