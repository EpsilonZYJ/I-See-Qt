#include "SettingsDialog.h"
#include "const/AppConfig.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    loadSettings();
}

SettingsDialog::~SettingsDialog() {
    // Qt 对象树自动清理
}

void SettingsDialog::setupUi() {
    setWindowTitle("设置");
    setModal(true);
    setMinimumWidth(600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // API Key 设置
    QGroupBox *apiKeyGroup = new QGroupBox("API 密钥");
    QVBoxLayout *apiKeyLayout = new QVBoxLayout;

    apiKeyEdit = new QLineEdit;
    apiKeyEdit->setPlaceholderText("请输入 API Key (Bearer Token)");
    apiKeyEdit->setEchoMode(QLineEdit::Password);

    QPushButton *showHideBtn = new QPushButton("显示/隐藏");
    showHideBtn->setMaximumWidth(100);
    connect(showHideBtn, &QPushButton::clicked, this, [this]() {
        if (apiKeyEdit->echoMode() == QLineEdit::Password) {
            apiKeyEdit->setEchoMode(QLineEdit::Normal);
        } else {
            apiKeyEdit->setEchoMode(QLineEdit::Password);
        }
    });

    QHBoxLayout *keyLayout = new QHBoxLayout;
    keyLayout->addWidget(apiKeyEdit);
    keyLayout->addWidget(showHideBtn);

    apiKeyLayout->addWidget(new QLabel("API Key:"));
    apiKeyLayout->addLayout(keyLayout);
    apiKeyGroup->setLayout(apiKeyLayout);

    // API 端点设置
    QGroupBox *apiEndpointGroup = new QGroupBox("API 端点配置");
    QVBoxLayout *endpointLayout = new QVBoxLayout;

    QLabel *tipLabel = new QLabel("提示：留空使用默认的官方 API");
    tipLabel->setStyleSheet("color: gray; font-style: italic;");
    endpointLayout->addWidget(tipLabel);

    // 提交 API
    endpointLayout->addWidget(new QLabel("提交 API URL:"));
    submitUrlEdit = new QLineEdit;
    submitUrlEdit->setPlaceholderText("默认: " + Config::SUBMIT_URL);
    endpointLayout->addWidget(submitUrlEdit);

    QLabel *submitDefaultLabel = new QLabel("默认值: " + Config::SUBMIT_URL);
    submitDefaultLabel->setStyleSheet("color: gray; font-size: 10px;");
    submitDefaultLabel->setWordWrap(true);
    endpointLayout->addWidget(submitDefaultLabel);

    endpointLayout->addSpacing(10);

    // 查询 API
    endpointLayout->addWidget(new QLabel("查询 API URL:"));
    queryUrlEdit = new QLineEdit;
    queryUrlEdit->setPlaceholderText("默认: " + Config::QUERY_URL);
    endpointLayout->addWidget(queryUrlEdit);

    QLabel *queryDefaultLabel = new QLabel("默认值: " + Config::QUERY_URL);
    queryDefaultLabel->setStyleSheet("color: gray; font-size: 10px;");
    queryDefaultLabel->setWordWrap(true);
    endpointLayout->addWidget(queryDefaultLabel);

    apiEndpointGroup->setLayout(endpointLayout);

    // 状态标签
    statusLabel = new QLabel("");
    statusLabel->setStyleSheet("color: green; font-weight: bold;");
    statusLabel->setAlignment(Qt::AlignCenter);

    // 按钮组
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    saveBtn = new QPushButton("保存");
    saveBtn->setMinimumHeight(35);
    saveBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");

    resetBtn = new QPushButton("重置为默认");
    resetBtn->setMinimumHeight(35);

    cancelBtn = new QPushButton("取消");
    cancelBtn->setMinimumHeight(35);

    buttonLayout->addWidget(resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(saveBtn);

    // 组装主布局
    mainLayout->addWidget(apiKeyGroup);
    mainLayout->addWidget(apiEndpointGroup);
    mainLayout->addWidget(statusLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(resetBtn, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
}

void SettingsDialog::loadSettings() {
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);

    // 加载 API Key
    QString apiKey = settings.value(Config::KEY_API_TOKEN).toString();
    apiKeyEdit->setText(apiKey);

    // 加载 API URLs
    QString submitUrl = settings.value("submitUrl", Config::SUBMIT_URL).toString();
    QString queryUrl = settings.value("queryUrl", Config::QUERY_URL).toString();

    // 如果是默认值则显示为空
    submitUrlEdit->setText(submitUrl == Config::SUBMIT_URL ? "" : submitUrl);
    queryUrlEdit->setText(queryUrl == Config::QUERY_URL ? "" : queryUrl);
}

void SettingsDialog::saveSettings() {
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);

    // 保存 API Key
    settings.setValue(Config::KEY_API_TOKEN, apiKeyEdit->text().trimmed());

    // 保存 API URLs（留空则保存默认值）
    QString submitUrl = submitUrlEdit->text().trimmed();
    QString queryUrl = queryUrlEdit->text().trimmed();

    settings.setValue("submitUrl", submitUrl.isEmpty() ? Config::SUBMIT_URL : submitUrl);
    settings.setValue("queryUrl", queryUrl.isEmpty() ? Config::QUERY_URL : queryUrl);

    qDebug() << "Settings saved";
}

QString SettingsDialog::getApiKey() const {
    return apiKeyEdit->text().trimmed();
}

QString SettingsDialog::getSubmitUrl() const {
    QString url = submitUrlEdit->text().trimmed();
    return url.isEmpty() ? Config::SUBMIT_URL : url;
}

QString SettingsDialog::getQueryUrl() const {
    QString url = queryUrlEdit->text().trimmed();
    return url.isEmpty() ? Config::QUERY_URL : url;
}

void SettingsDialog::onSaveClicked() {
    // 验证 API Key
    if (apiKeyEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入 API Key");
        return;
    }

    // 保存设置
    saveSettings();

    // 显示成功消息
    statusLabel->setText("✓ 设置已保存并立即生效");
    statusLabel->setStyleSheet("color: green; font-weight: bold;");

    // 发射设置变化信号
    emit settingsChanged();

    // 延迟关闭对话框
    QTimer::singleShot(800, this, &SettingsDialog::accept);
}

void SettingsDialog::onCancelClicked() {
    reject();
}

void SettingsDialog::onResetClicked() {
    auto reply = QMessageBox::question(this, "确认重置",
        "确定要重置所有 API 设置为默认值吗？\n\nAPI Key 将被清空。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 清空 API Key
        apiKeyEdit->clear();

        // 清空自定义 URL
        submitUrlEdit->clear();
        queryUrlEdit->clear();

        statusLabel->setText("已重置为默认值（未保存）");
        statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    }
}

