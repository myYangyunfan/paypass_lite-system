/* ============================================
   paypass_lite (交易轻) — Frontend Logic
   ============================================ */

const API_BASE = '/api';

// ==================== i18n ====================
var lang = 'en';

var i18n = {
  en: {
    pageTitle: 'paypass_lite (交易轻)',
    brandName: 'paypass_lite',
    navCreateAccount: 'Create Account',
    navDeposit: 'Deposit',
    navWithdraw: 'Withdraw',
    navTransfer: 'Transfer',
    navSearch: 'Search',
    navViewLedger: 'View Ledger',
    navUndo: 'Undo',
    navDelete: 'Delete',
    titleCreateAccount: 'Create Account',
    titleDeposit: 'Deposit Money',
    titleWithdraw: 'Withdraw Money',
    titleTransfer: 'Transfer Money',
    titleSearch: 'Search Account',
    titleViewLedger: 'View Ledger',
    titleUndo: 'Undo Last Transaction',
    titleDelete: 'Delete Account',
    labelAccountNumber: 'Account Number',
    labelUserName: 'User Name',
    labelPhoneNumber: 'Phone Number',
    labelAmount: 'Amount',
    labelFromAccount: 'From Account',
    labelToAccount: 'To Account',
    placeholderAccNum: 'e.g. ACC001',
    placeholderUserName: 'e.g. Alice',
    placeholderPhone: 'e.g. 13800001111',
    placeholderAmount: '0.00',
    placeholderFrom: 'Source account',
    placeholderTo: 'Destination account',
    btnCreateAccount: 'Create Account',
    btnDeposit: 'Deposit',
    btnWithdraw: 'Withdraw',
    btnTransfer: 'Transfer',
    btnSearch: 'Search',
    btnViewLedger: 'View Ledger',
    btnUndo: 'Undo Last Transaction',
    btnDelete: 'Delete Account',
    descUndo: 'This will reverse the most recent transaction (deposit, withdrawal, or transfer).',
    processing: 'Processing...',
    undoing: 'Undoing...',
    success: 'Success',
    networkError: 'Network error — cannot reach server. Is paypass_server running?',
    noTransactions: 'No transactions yet.',
    thId: 'ID',
    thType: 'Type',
    thAmount: 'Amount',
    thFrom: 'From',
    thTo: 'To',
    thTime: 'Time',
    labelAccount: 'Account',
    labelBalance: 'Balance',
    labelAccountColon: 'Account:',
    labelBalanceAfter: 'Balance after:',
    labelHolder: 'Holder:',
    labelFromAfter: 'From (after):',
    labelToAfter: 'To (after):',
  },
  zh: {
    pageTitle: 'paypass_lite (交易轻)',
    brandName: '交易轻',
    navCreateAccount: '创建账户',
    navDeposit: '存款',
    navWithdraw: '取款',
    navTransfer: '转账',
    navSearch: '查询',
    navViewLedger: '查看账本',
    navUndo: '撤销',
    navDelete: '删除',
    titleCreateAccount: '创建账户',
    titleDeposit: '存款',
    titleWithdraw: '取款',
    titleTransfer: '转账',
    titleSearch: '查询账户',
    titleViewLedger: '查看账本',
    titleUndo: '撤销最后一笔交易',
    titleDelete: '删除账户',
    labelAccountNumber: '账号',
    labelUserName: '用户名',
    labelPhoneNumber: '电话',
    labelAmount: '金额',
    labelFromAccount: '来源账户',
    labelToAccount: '目标账户',
    placeholderAccNum: '例如：ACC001',
    placeholderUserName: '例如：张三',
    placeholderPhone: '例如：13800001111',
    placeholderAmount: '0.00',
    placeholderFrom: '来源账号',
    placeholderTo: '目标账号',
    btnCreateAccount: '创建账户',
    btnDeposit: '存款',
    btnWithdraw: '取款',
    btnTransfer: '转账',
    btnSearch: '查询',
    btnViewLedger: '查看账本',
    btnUndo: '撤销最后一笔交易',
    btnDelete: '删除账户',
    descUndo: '这将撤销最近的一笔交易（存款、取款或转账）。',
    processing: '处理中...',
    undoing: '撤销中...',
    success: '成功',
    networkError: '网络错误 — 无法连接服务器。请确认 paypass_server 正在运行。',
    noTransactions: '暂无交易记录。',
    thId: '编号',
    thType: '类型',
    thAmount: '金额',
    thFrom: '来源',
    thTo: '目标',
    thTime: '时间',
    labelAccount: '账号',
    labelBalance: '余额',
    labelAccountColon: '账号：',
    labelBalanceAfter: '操作后余额：',
    labelHolder: '持有人：',
    labelFromAfter: '来源方（操作后）：',
    labelToAfter: '目标方（操作后）：',
  }
};

var serverMessages = {
  en: {
    "Created": "Created",
    "Deposit OK": "Deposit OK",
    "Withdraw OK": "Withdraw OK",
    "Transfer OK": "Transfer OK",
    "Found": "Found",
    "Undone": "Undone",
    "Deleted": "Deleted",
    "Nothing to undo": "Nothing to undo",
    "Missing account_number or user_name": "Missing account_number or user_name",
    "Fields cannot be empty": "Fields cannot be empty",
    "Account exists": "Account exists",
    "Invalid JSON": "Invalid JSON",
    "Missing amount": "Missing amount",
    "Positive amount required": "Positive amount required",
    "Account not found": "Account not found",
    "Insufficient balance": "Insufficient balance",
    "Missing from/to/amount": "Missing from/to/amount",
    "Cannot self-transfer": "Cannot self-transfer",
    "Transfer failed": "Transfer failed",
    "Not found": "Not found"
  },
  zh: {
    "Created": "创建成功",
    "Deposit OK": "存款成功",
    "Withdraw OK": "取款成功",
    "Transfer OK": "转账成功",
    "Found": "已找到",
    "Undone": "已撤销",
    "Deleted": "已删除",
    "Nothing to undo": "没有可撤销的操作",
    "Missing account_number or user_name": "缺少账号或用户名",
    "Fields cannot be empty": "字段不能为空",
    "Account exists": "账号已存在",
    "Invalid JSON": "无效的JSON",
    "Missing amount": "缺少金额",
    "Positive amount required": "金额必须为正数",
    "Account not found": "账号未找到",
    "Insufficient balance": "余额不足",
    "Missing from/to/amount": "缺少来源/目标/金额",
    "Cannot self-transfer": "不能转账给自己",
    "Transfer failed": "转账失败",
    "Not found": "未找到"
  }
};

function t(key) {
  return (i18n[lang] && i18n[lang][key]) || key;
}

function translateServerMessage(msg) {
  return (serverMessages[lang] && serverMessages[lang][msg]) || msg;
}

function switchLang() {
  lang = (lang === 'en') ? 'zh' : 'en';
  updateI18n();
}

function updateI18n() {
  document.querySelectorAll('[data-i18n]').forEach(function(el) {
    var key = el.getAttribute('data-i18n');
    el.textContent = t(key);
  });
  document.querySelectorAll('[data-i18n-placeholder]').forEach(function(el) {
    var key = el.getAttribute('data-i18n-placeholder');
    el.placeholder = t(key);
  });
  var btn = document.getElementById('btn-lang');
  if (btn) {
    btn.textContent = lang === 'en' ? t('switchToChinese') : t('switchToEnglish');
  }
  document.title = t('pageTitle');
}

document.addEventListener('DOMContentLoaded', function() {
  updateI18n();
  document.getElementById('btn-lang').addEventListener('click', switchLang);
});

// ==================== Toast ====================
function showToast(message, type) {
  var container = document.getElementById('toast-container');
  var toast = document.createElement('div');
  toast.className = 'toast toast-' + type;
  toast.textContent = message;
  container.appendChild(toast);
  setTimeout(function() { toast.remove(); }, 3300);
}

// ==================== API Request ====================
async function apiRequest(method, url, data) {
  var options = {
    method: method,
    headers: { 'Content-Type': 'application/json' }
  };
  if (data) options.body = JSON.stringify(data);

  try {
    var res = await fetch(API_BASE + url, options);
    var json = await res.json();
    if (!res.ok) {
      throw new Error(translateServerMessage(json.error || json.message || 'Request failed (HTTP ' + res.status + ')'));
    }
    return json;
  } catch (err) {
    if (err.message === 'Failed to fetch') {
      throw new Error(t('networkError'));
    }
    throw err;
  }
}

// ==================== Panel Switching ====================
document.querySelectorAll('.nav-item').forEach(function(item) {
  item.addEventListener('click', function() {
    document.querySelectorAll('.nav-item').forEach(function(n) { n.classList.remove('active'); });
    item.classList.add('active');
    var panelId = 'panel-' + item.getAttribute('data-panel');
    document.querySelectorAll('.panel').forEach(function(p) { p.classList.remove('active'); });
    var panel = document.getElementById(panelId);
    if (panel) panel.classList.add('active');
  });
});

// ==================== Rendering Helpers ====================

function renderAccountCard(account, containerId) {
  var div = document.getElementById(containerId);
  div.innerHTML = '<div class="result-card">' +
    '<dl class="account-card">' +
      '<dt>' + t('labelAccount') + '</dt><dd>' + esc(account.account_number) + '</dd>' +
      '<dt>' + t('labelUserName') + '</dt><dd>' + esc(account.user_name) + '</dd>' +
      '<dt>' + t('labelPhoneNumber') + '</dt><dd>' + esc(account.phone_number || '-') + '</dd>' +
      '<dt>' + t('labelBalance') + '</dt><dd class="balance-highlight">' + fmtMoney(account.balance) + '</dd>' +
    '</dl></div>';
}

function renderAccountSummary(account, containerId) {
  var div = document.getElementById(containerId);
  div.innerHTML = '<div class="result-card">' +
    '<p style="margin-bottom:12px;color:var(--text-secondary)">' + t('labelAccountColon') + ' <strong>' +
    esc(account.account_number) + '</strong> | ' + t('labelBalanceAfter') + ' <span class="balance-highlight">' +
    fmtMoney(account.balance) + '</span></p></div>';
}

function renderLedger(data, containerId) {
  var div = document.getElementById(containerId);
  var txns = data.transactions || [];
  var html = '<div class="result-card">' +
    '<p style="margin-bottom:12px">' + t('labelAccountColon') + ' <strong>' + esc(data.account_number) +
    '</strong> | ' + t('labelHolder') + ' <strong>' + esc(data.user_name) +
    '</strong> | <span class="balance-highlight">' + fmtMoney(data.balance) + '</span></p>';

  if (txns.length === 0) {
    html += '<p class="no-data">' + t('noTransactions') + '</p>';
  } else {
    html += '<table class="txn-table"><thead><tr>' +
      '<th>' + t('thId') + '</th><th>' + t('thType') + '</th><th>' + t('thAmount') + '</th><th>' + t('thFrom') + '</th><th>' + t('thTo') + '</th><th>' + t('thTime') + '</th>' +
      '</tr></thead><tbody>';
    txns.forEach(function(t) {
      html += '<tr>' +
        '<td>' + t.txn_id + '</td>' +
        '<td><span class="txn-type ' + t.type.toLowerCase() + '">' + t.type + '</span></td>' +
        '<td class="' + (t.type === 'DEPOSIT' ? 'amount-positive' : 'amount-negative') + '">' + fmtMoney(t.amount) + '</td>' +
        '<td>' + (t.from ? esc(t.from) : '-') + '</td>' +
        '<td>' + (t.to   ? esc(t.to)   : '-') + '</td>' +
        '<td>' + esc(t.timestamp) + '</td>' +
        '</tr>';
    });
    html += '</tbody></table>';
  }
  html += '</div>';
  div.innerHTML = html;
}

function renderTransferResult(json, containerId) {
  var div = document.getElementById(containerId);
  var fa = json.from_account;
  var ta = json.to_account;
  div.innerHTML = '<div class="result-card">' +
    '<div style="display:flex;gap:24px;flex-wrap:wrap">' +
      '<div style="flex:1;min-width:200px">' +
        '<p style="color:var(--text-secondary);margin-bottom:8px">' + t('labelFromAfter') + '</p>' +
        '<dl class="account-card"><dt>' + t('labelAccount') + '</dt><dd>' + esc(fa.account_number) + '</dd>' +
        '<dt>' + t('labelBalance') + '</dt><dd class="balance-highlight">' + fmtMoney(fa.balance) + '</dd></dl>' +
      '</div>' +
      '<div style="flex:1;min-width:200px">' +
        '<p style="color:var(--text-secondary);margin-bottom:8px">' + t('labelToAfter') + '</p>' +
        '<dl class="account-card"><dt>' + t('labelAccount') + '</dt><dd>' + esc(ta.account_number) + '</dd>' +
        '<dt>' + t('labelBalance') + '</dt><dd class="balance-highlight">' + fmtMoney(ta.balance) + '</dd></dl>' +
      '</div>' +
    '</div></div>';
}

function renderMessage(msg, type, containerId) {
  var div = document.getElementById(containerId);
  div.innerHTML = '<div class="result-card" style="color:' +
    (type === 'error' ? 'var(--danger)' : 'var(--success)') + ';font-weight:500">' +
    esc(msg) + '</div>';
}

function esc(s) {
  if (!s) return '';
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

function fmtMoney(n) {
  return Number(n).toLocaleString('zh-CN', { style: 'currency', currency: 'CNY' });
}

// ==================== Generic Form Handler ====================
function bindForm(formId, containerId, buildRequest) {
  var form = document.getElementById(formId);
  if (!form) return;
  form.addEventListener('submit', async function(e) {
    e.preventDefault();
    var container = document.getElementById(containerId);
    var btn = form.querySelector('button[type="submit"]');
    var origText = btn.textContent;
    btn.disabled = true;
    btn.innerHTML = '<span class="spinner"></span> ' + t('processing');

    try {
      var req = buildRequest(form);
      var json = await apiRequest(req.method, req.url, req.body);
      showToast(translateServerMessage(json.message) || t('success'), 'success');
      if (req.render) req.render(json, containerId);
    } catch (err) {
      showToast(err.message, 'error');
      renderMessage(err.message, 'error', containerId);
    } finally {
      btn.disabled = false;
      btn.textContent = origText;
    }
  });
}

// ==================== Form Bindings ====================

// 1. Create Account
bindForm('form-create-account', 'result-create-account', function(form) {
  var data = {
    account_number: form.account_number.value.trim(),
    user_name: form.user_name.value.trim(),
    phone_number: form.phone_number.value.trim()
  };
  return {
    method: 'POST', url: '/accounts', body: data,
    render: function(json, cid) {
      if (json.account) renderAccountCard(json.account, cid);
    }
  };
});

// 2. Deposit
bindForm('form-deposit', 'result-deposit', function(form) {
  var accNum = form.account_number.value.trim();
  var amount = parseFloat(form.amount.value);
  return {
    method: 'POST', url: '/accounts/' + encodeURIComponent(accNum) + '/deposit',
    body: { amount: amount },
    render: function(json, cid) {
      if (json.account) renderAccountSummary(json.account, cid);
      else renderMessage(translateServerMessage(json.message || 'Deposit OK'), 'success', cid);
    }
  };
});

// 3. Withdraw
bindForm('form-withdraw', 'result-withdraw', function(form) {
  var accNum = form.account_number.value.trim();
  var amount = parseFloat(form.amount.value);
  return {
    method: 'POST', url: '/accounts/' + encodeURIComponent(accNum) + '/withdraw',
    body: { amount: amount },
    render: function(json, cid) {
      if (json.account) renderAccountSummary(json.account, cid);
      else renderMessage(translateServerMessage(json.message || 'Withdraw OK'), 'success', cid);
    }
  };
});

// 4. Transfer
bindForm('form-transfer', 'result-transfer', function(form) {
  return {
    method: 'POST', url: '/transfer',
    body: {
      from: form.from.value.trim(),
      to: form.to.value.trim(),
      amount: parseFloat(form.amount.value)
    },
    render: function(json, cid) {
      if (json.from_account && json.to_account) {
        renderTransferResult(json, cid);
      } else {
        renderMessage(translateServerMessage(json.message || 'Transfer OK'), 'success', cid);
      }
    }
  };
});

// 5. Search
bindForm('form-search', 'result-search', function(form) {
  var accNum = form.account_number.value.trim();
  return {
    method: 'GET', url: '/accounts/' + encodeURIComponent(accNum),
    render: function(json, cid) {
      if (json.account) renderAccountCard(json.account, cid);
    }
  };
});

// 6. View Ledger
bindForm('form-ledger', 'result-ledger', function(form) {
  var accNum = form.account_number.value.trim();
  return {
    method: 'GET', url: '/accounts/' + encodeURIComponent(accNum) + '/ledger',
    render: function(json, cid) { renderLedger(json, cid); }
  };
});

// 7. Undo (button, not form)
document.getElementById('btn-undo').addEventListener('click', async function() {
  var btn = this;
  var origText = btn.textContent;
  btn.disabled = true;
  btn.innerHTML = '<span class="spinner"></span> ' + t('undoing');

  try {
    var json = await apiRequest('POST', '/undo');
    showToast(translateServerMessage(json.message) || t('success'), 'success');
    renderMessage(translateServerMessage(json.message) || t('success'), 'success', 'result-undo');
  } catch (err) {
    showToast(err.message, 'error');
    renderMessage(err.message, 'error', 'result-undo');
  } finally {
    btn.disabled = false;
    btn.textContent = origText;
  }
});

// 8. Delete
bindForm('form-delete-account', 'result-delete-account', function(form) {
  var accNum = form.account_number.value.trim();
  return {
    method: 'DELETE', url: '/accounts/' + encodeURIComponent(accNum),
    render: function(json, cid) { renderMessage(translateServerMessage(json.message) || 'Deleted', 'success', cid); }
  };
});
