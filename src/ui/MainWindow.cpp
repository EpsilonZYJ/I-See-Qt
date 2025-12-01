#include "MainWindow.h"
#include "TaskHistoryWindow.h"
#include "const/QtHeaders.h"
#include "const/AppConfig.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), taskHistoryWindow(nullptr) {
    viewModel = new MainViewModel(this);

    // 初始化 UI 布局
    setupUi();

    // 加载配置中的 API Key
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    apiKeyEdit->setText(settings.value(Config::KEY_API_TOKEN).toString());

    // --- 信号与槽的绑定 ---

    // 1. ViewModel -> UI
    connect(viewModel, &MainViewModel::statusChanged, statusLabel, &QLabel::setText);
    connect(viewModel, &MainViewModel::progressUpdated, progressBar, &QProgressBar::setValue);

    connect(viewModel, &MainViewModel::errorOccurred, this, [this](const QString &msg){
        QMessageBox::warning(this, "错误", msg);
        generateBtn->setEnabled(true);
        statusLabel->setText("发生错误");
    });

    connect(viewModel, &MainViewModel::videoReady, this, &MainWindow::onVideoReady);
    connect(viewModel, &MainViewModel::historyUpdated, this, &MainWindow::updateHistoryList);

    // 2. UI -> UI/ViewModel
    connect(generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
    connect(taskHistoryBtn, &QPushButton::clicked, this, &MainWindow::onShowTaskHistory);
    connect(addParameterBtn, &QPushButton::clicked, this, [this]() {
        addParameterRow("", "");
    });

    // 列表点击播放
    connect(historyList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        int row = historyList->row(item);
        auto items = viewModel->getHistory();
        if(row >= 0 && row < items.size()) {
            player->setSource(QUrl::fromLocalFile(items[row].filePath));
            player->play();
        }
    });

    // 初始加载历史
    viewModel->loadHistory();
}

// 析构函数实现 (解决 Undefined symbol: ~MainWindow)
MainWindow::~MainWindow() {
    // Qt 的对象树机制会自动清理子对象(new出来的控件)
    // 这里主要是为了保存非实时保存的状态
    if (taskHistoryWindow) {
        delete taskHistoryWindow;
    }
}

// 布局实现 (解决 Undefined symbol: setupUi)
void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // --- 左侧：历史记录 ---
    historyList = new QListWidget;
    historyList->setFixedWidth(220);

    // --- 右侧：控制区与预览 ---
    QWidget *rightWidget = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);

    // 1. 设置区域
    QGroupBox *settingsBox = new QGroupBox("设置");
    QVBoxLayout *settingsLayout = new QVBoxLayout;
    apiKeyEdit = new QLineEdit;
    apiKeyEdit->setPlaceholderText("API Key (Bearer Token)");
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    settingsLayout->addWidget(apiKeyEdit);
    settingsBox->setLayout(settingsLayout);

    // 1.5 参数配置区域
    QGroupBox *parametersBox = new QGroupBox("参数配置");
    QVBoxLayout *parametersBoxLayout = new QVBoxLayout;

    // 参数列表容器
    QScrollArea *parametersScroll = new QScrollArea;
    parametersScroll->setWidgetResizable(true);
    parametersScroll->setMaximumHeight(200);

    parametersWidget = new QWidget;
    parametersLayout = new QVBoxLayout(parametersWidget);
    parametersLayout->setSpacing(5);
    parametersLayout->addStretch();

    parametersScroll->setWidget(parametersWidget);

    // 添加参数按钮
    addParameterBtn = new QPushButton("+ 添加参数");
    addParameterBtn->setMaximumWidth(120);

    parametersBoxLayout->addWidget(parametersScroll);
    parametersBoxLayout->addWidget(addParameterBtn);
    parametersBox->setLayout(parametersBoxLayout);

    // 添加默认参数
    addParameterRow("width", "1280");
    addParameterRow("height", "720");
    addParameterRow("resolution", "1080p");
    addParameterRow("aspect_ratio", "16:9");
    addParameterRow("duration", "5");
    addParameterRow("camera_fixed", "false");
    addParameterRow("seed", "123");

    // 2. 输入区域
    promptEdit = new QTextEdit;
    promptEdit->setPlaceholderText("请输入提示词...");
    promptEdit->setMaximumHeight(80);
    promptEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
    promptEdit->setInputMethodHints(Qt::ImhNone);
    promptEdit->setFont(QFont("", 12));

    // 3. 按钮与状态
    generateBtn = new QPushButton("生成视频");
    generateBtn->setMinimumHeight(40);

    taskHistoryBtn = new QPushButton("查看任务历史");
    taskHistoryBtn->setMinimumHeight(40);

    progressBar = new QProgressBar;
    progressBar->setValue(0);
    progressBar->setTextVisible(false);

    statusLabel = new QLabel("准备就绪");
    statusLabel->setStyleSheet("color: gray; font-size: 12px;");
    statusLabel->setAlignment(Qt::AlignCenter);

    // 4. 视频播放器 (Qt6)
    videoWidget = new QVideoWidget;
    videoWidget->setMinimumHeight(300);
    videoWidget->setStyleSheet("background-color: black;");

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    player->setVideoOutput(videoWidget);

    // 组装右侧
    rightLayout->addWidget(settingsBox);
    rightLayout->addWidget(parametersBox);
    rightLayout->addWidget(new QLabel("Prompt:"));
    rightLayout->addWidget(promptEdit);
    rightLayout->addWidget(generateBtn);
    rightLayout->addWidget(taskHistoryBtn);
    rightLayout->addWidget(progressBar);
    rightLayout->addWidget(statusLabel);
    rightLayout->addWidget(videoWidget, 1); // 1 表示占据剩余空间

    // 组装整体
    mainLayout->addWidget(historyList);
    mainLayout->addWidget(rightWidget);

    resize(1000, 700);
    setWindowTitle(Config::APP_NAME);
}

void MainWindow::onGenerateClicked() {
    QString key = apiKeyEdit->text();
    if(key.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入 API Key");
        return;
    }

    QString prompt = promptEdit->toPlainText();
    if(prompt.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入提示词");
        return;
    }

    // 保存 Key
    QSettings s(Config::ORG_NAME, Config::APP_NAME);
    s.setValue(Config::KEY_API_TOKEN, key);

    // 获取用户配置的参数
    QMap<QString, QString> params = getParameters();

    generateBtn->setEnabled(false);
    viewModel->startGeneration(key, prompt, params);
}

void MainWindow::onVideoReady(const QString &path) {
    generateBtn->setEnabled(true);
    player->setSource(QUrl::fromLocalFile(path));
    player->play();
}

void MainWindow::updateHistoryList() {
    historyList->clear();
    auto items = viewModel->getHistory();
    for(const auto &item : items) {
        // 只显示 prompt 前30个字符和日期
        QString label = item.prompt.left(30) + (item.prompt.length()>30?"...":"") + "\n" + item.date;
        historyList->addItem(label);
    }
}

void MainWindow::onShowTaskHistory() {
    if (!taskHistoryWindow) {
        // 从 ViewModel 获取服务实例
        TaskDatabaseService *dbService = viewModel->getTaskDatabaseService();
        ApiService *apiService = new ApiService(this); // 创建一个独立的 ApiService 用于任务历史窗口

        taskHistoryWindow = new TaskHistoryWindow(dbService, apiService, this);

        // 连接 ApiService 的 taskPolled 信号到窗口
        connect(apiService, &ApiService::taskPolled, taskHistoryWindow, &TaskHistoryWindow::onTaskPolled);
    }

    taskHistoryWindow->show();
    taskHistoryWindow->raise();
    taskHistoryWindow->activateWindow();
    taskHistoryWindow->refreshTasks();
}

void MainWindow::addParameterRow(const QString &name, const QString &value) {
    ParameterRow row;

    // 创建水平布局
    row.layout = new QHBoxLayout;
    row.layout->setSpacing(5);

    // 参数名输入框
    row.nameEdit = new QLineEdit;
    row.nameEdit->setPlaceholderText("参数名 (如: width)");
    row.nameEdit->setText(name);
    row.nameEdit->setMaximumWidth(150);

    // 参数值输入框
    row.valueEdit = new QLineEdit;
    row.valueEdit->setPlaceholderText("参数值 (如: 1280)");
    row.valueEdit->setText(value);
    row.valueEdit->setMaximumWidth(150);

    // 删除按钮
    row.removeBtn = new QPushButton("×");
    row.removeBtn->setMaximumWidth(30);
    row.removeBtn->setStyleSheet("QPushButton { color: red; font-weight: bold; }");

    // 添加到布局
    row.layout->addWidget(row.nameEdit);
    row.layout->addWidget(new QLabel("="));
    row.layout->addWidget(row.valueEdit);
    row.layout->addWidget(row.removeBtn);
    row.layout->addStretch();

    // 保存到列表
    int index = parameterRows.size();
    parameterRows.append(row);

    // 连接删除按钮
    connect(row.removeBtn, &QPushButton::clicked, this, [this, index]() {
        removeParameterRow(index);
    });

    // 插入到布局中（在 stretch 之前）
    parametersLayout->insertLayout(parametersLayout->count() - 1, row.layout);
}

void MainWindow::removeParameterRow(int index) {
    if (index < 0 || index >= parameterRows.size()) {
        return;
    }

    ParameterRow &row = parameterRows[index];

    // 从布局中移除
    parametersLayout->removeItem(row.layout);

    // 删除所有控件
    delete row.nameEdit;
    delete row.valueEdit;
    delete row.removeBtn;

    // 删除布局中的 QLabel
    while (row.layout->count() > 0) {
        QLayoutItem *item = row.layout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    delete row.layout;

    // 从列表中移除
    parameterRows.removeAt(index);

    // 重新连接剩余按钮的索引
    for (int i = index; i < parameterRows.size(); ++i) {
        ParameterRow &r = parameterRows[i];
        disconnect(r.removeBtn, nullptr, nullptr, nullptr);
        connect(r.removeBtn, &QPushButton::clicked, this, [this, i]() {
            removeParameterRow(i);
        });
    }
}

QMap<QString, QString> MainWindow::getParameters() const {
    QMap<QString, QString> params;

    for (const ParameterRow &row : parameterRows) {
        QString name = row.nameEdit->text().trimmed();
        QString value = row.valueEdit->text().trimmed();

        if (!name.isEmpty() && !value.isEmpty()) {
            params[name] = value;
        }
    }

    return params;
}

