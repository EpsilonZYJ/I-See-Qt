#include "MainWindow.h"
#include "TaskHistoryWindow.h"
#include "SettingsDialog.h"
#include "const/QtHeaders.h"
#include "const/AppConfig.h"
#include "services/TaskDatabaseService.h"
#include "models/TaskItem.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), taskHistoryWindow(nullptr), settingsDialog(nullptr) {
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
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onShowSettings);
    connect(addParameterBtn, &QPushButton::clicked, this, [this]() {
        addParameterRow("", "");
    });

    // 列表点击播放
    connect(historyList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        int row = historyList->row(item);
        auto items = viewModel->getHistory();
        if(row >= 0 && row < items.size()) {
            QString filePath = items[row].filePath;

            // 检查视频文件是否存在
            if (QFile::exists(filePath)) {
                // 文件存在，直接播放
                player->setSource(QUrl::fromLocalFile(filePath));
                player->play();
            } else {
                // 文件不存在，尝试重新下载
                statusLabel->setText("视频文件不存在，正在重新下载...");

                // 从文件路径提取 task_id（格式：taskId_timestamp.mp4）
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.fileName();
                QString taskId = extractTaskIdFromFileName(fileName);

                if (!taskId.isEmpty()) {
                    // 从数据库查找任务
                    TaskDatabaseService *dbService = viewModel->getTaskDatabaseService();
                    TaskItem task = dbService->getTask(taskId);

                    if (!task.taskId.isEmpty() && !task.videoUrl.isEmpty()) {
                        // 找到任务且有视频 URL，重新下载
                        qDebug() << "重新下载视频，Task ID:" << taskId;
                        statusLabel->setText(QString("正在重新下载视频... (Task ID: %1)").arg(taskId));

                        // 使用 ApiService 下载
                        ApiService *downloadService = new ApiService(this);
                        connect(downloadService, &ApiService::videoDownloaded, this, [this, taskId, filePath, downloadService](const QString &tempPath) {
                            // 下载完成，移动到原位置
                            QFile::remove(filePath); // 删除可能存在的旧文件
                            if (QFile::rename(tempPath, filePath)) {
                                statusLabel->setText("视频下载完成");
                                player->setSource(QUrl::fromLocalFile(filePath));
                                player->play();

                                // 更新数据库
                                TaskDatabaseService *dbService = viewModel->getTaskDatabaseService();
                                TaskItem task = dbService->getTask(taskId);
                                task.localFilePath = filePath;
                                task.updateTime = QDateTime::currentDateTime();
                                dbService->updateTask(task);
                            } else {
                                statusLabel->setText("视频文件移动失败");
                            }
                            downloadService->deleteLater();
                        });

                        connect(downloadService, &ApiService::errorOccurred, this, [this, downloadService](const QString &error) {
                            statusLabel->setText("视频下载失败: " + error);
                            downloadService->deleteLater();
                        });

                        downloadService->downloadVideo(task.videoUrl);
                    } else {
                        QMessageBox::warning(this, "提示",
                            QString("无法重新下载视频\nTask ID: %1\n请在任务历史中重新查询该任务").arg(taskId));
                        statusLabel->setText("视频文件不存在");
                    }
                } else {
                    QMessageBox::warning(this, "提示", "无法从文件名提取 Task ID，无法重新下载");
                    statusLabel->setText("视频文件不存在");
                }
            }
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

    // API Key 显示（只读）
    apiKeyEdit = new QLineEdit;
    apiKeyEdit->setPlaceholderText("未设置 API Key");
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setReadOnly(true);  // 设置为只读
    apiKeyEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; }");

    QHBoxLayout *keyLayout = new QHBoxLayout;
    keyLayout->addWidget(new QLabel("API Key:"));
    keyLayout->addWidget(apiKeyEdit);

    // 设置按钮
    settingsBtn = new QPushButton("⚙ 设置");
    settingsBtn->setMinimumHeight(35);
    settingsBtn->setMaximumWidth(100);
    settingsBtn->setStyleSheet("QPushButton { font-weight: bold; }");
    keyLayout->addWidget(settingsBtn);

    settingsLayout->addLayout(keyLayout);
    settingsBox->setLayout(settingsLayout);

    // 1.25 模式选择区域
    QGroupBox *modeBox = new QGroupBox("生成模式");
    QHBoxLayout *modeLayout = new QHBoxLayout;

    modeSelector = new QComboBox;
    modeSelector->addItem("文生视频 (Text-to-Video)");
    modeSelector->addItem("图生视频 (Image-to-Video)");
    modeLayout->addWidget(new QLabel("模式:"));
    modeLayout->addWidget(modeSelector);
    modeLayout->addStretch();

    modeBox->setLayout(modeLayout);

    connect(modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);

    // 1.3 图片输入区域（初始隐藏）
    imageInputWidget = new QWidget;
    QVBoxLayout *imageInputLayout = new QVBoxLayout(imageInputWidget);

    // 首帧图片
    QHBoxLayout *firstImageLayout = new QHBoxLayout;
    selectImageBtn = new QPushButton("选择首帧图片");
    selectImageBtn->setMaximumWidth(120);
    imagePreviewLabel = new QLabel("未选择图片");
    imagePreviewLabel->setStyleSheet("border: 1px solid #ccc; padding: 5px;");
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setMaximumHeight(80);
    imagePreviewLabel->setScaledContents(false);
    firstImageLayout->addWidget(selectImageBtn);
    firstImageLayout->addWidget(imagePreviewLabel, 1);

    // 尾帧图片（可选）
    QHBoxLayout *lastImageLayout = new QHBoxLayout;
    selectLastImageBtn = new QPushButton("选择尾帧图片(可选)");
    selectLastImageBtn->setMaximumWidth(120);
    lastImagePreviewLabel = new QLabel("未选择图片");
    lastImagePreviewLabel->setStyleSheet("border: 1px solid #ccc; padding: 5px;");
    lastImagePreviewLabel->setAlignment(Qt::AlignCenter);
    lastImagePreviewLabel->setMaximumHeight(80);
    lastImagePreviewLabel->setScaledContents(false);
    lastImageLayout->addWidget(selectLastImageBtn);
    lastImageLayout->addWidget(lastImagePreviewLabel, 1);

    imageInputLayout->addLayout(firstImageLayout);
    imageInputLayout->addLayout(lastImageLayout);

    imageInputWidget->setVisible(false);  // 初始隐藏

    connect(selectImageBtn, &QPushButton::clicked, this, &MainWindow::onSelectImage);
    connect(selectLastImageBtn, &QPushButton::clicked, this, &MainWindow::onSelectLastImage);

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
    rightLayout->addWidget(modeBox);
    rightLayout->addWidget(imageInputWidget);
    rightLayout->addWidget(parametersBox);
    rightLayout->addWidget(new QLabel("Prompt:"));
    rightLayout->addWidget(promptEdit);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setSpacing(10);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    generateBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    taskHistoryBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    buttonsLayout->addWidget(generateBtn);
    buttonsLayout->addWidget(taskHistoryBtn);
    rightLayout->addLayout(buttonsLayout);

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
    // 从设置中读取 API Key
    QSettings s(Config::ORG_NAME, Config::APP_NAME);
    QString key = s.value(Config::KEY_API_TOKEN).toString();

    if(key.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在设置中配置 API Key");
        return;
    }

    QString prompt = promptEdit->toPlainText();

    // 检查是文生视频还是图生视频
    bool isImageToVideo = (modeSelector->currentIndex() == 1);

    if (isImageToVideo) {
        // 图生视频模式
        if (firstImagePath.isEmpty()) {
            QMessageBox::warning(this, "提示", "请先选择首帧图片");
            return;
        }

        // Prompt 在图生视频中是可选的，但建议提供
        if (prompt.isEmpty()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "提示",
                "未输入提示词，是否继续？\n（建议提供提示词以获得更好的生成效果）",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::No) {
                return;
            }
        }

        // 转换图片为 Base64
        QString imageBase64 = imageToBase64(firstImagePath);
        if (imageBase64.isEmpty()) {
            QMessageBox::warning(this, "错误", "无法读取首帧图片");
            return;
        }

        QString lastImageBase64;
        if (!lastImagePath.isEmpty()) {
            lastImageBase64 = imageToBase64(lastImagePath);
            if (lastImageBase64.isEmpty()) {
                QMessageBox::warning(this, "错误", "无法读取尾帧图片");
                return;
            }
        }

        generateBtn->setEnabled(false);
        statusLabel->setText("正在提交图生视频任务...");
        viewModel->startImageToVideoGeneration(key, prompt, imageBase64, lastImageBase64);

    } else {
        // 文生视频模式
        if(prompt.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入提示词");
            return;
        }

        // 获取用户配置的参数
        QMap<QString, QString> params = getParameters();

        generateBtn->setEnabled(false);
        statusLabel->setText("正在提交文生视频任务...");
        viewModel->startGeneration(key, prompt, params);
    }
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

void MainWindow::onShowSettings() {
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(this);

        // 连接设置变化信号
        connect(settingsDialog, &SettingsDialog::settingsChanged, this, &MainWindow::onSettingsChanged);
    }

    settingsDialog->exec();
}

void MainWindow::onSettingsChanged() {
    // 重新加载 API Key 显示
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    QString apiKey = settings.value(Config::KEY_API_TOKEN).toString();
    apiKeyEdit->setText(apiKey);

    // 重新加载 API URLs
    viewModel->getApiService()->reloadApiUrls();

    statusLabel->setText("设置已更新并立即生效");

    qDebug() << "Settings changed and reloaded";
}

QString MainWindow::extractTaskIdFromFileName(const QString &fileName) const {
    // 视频文件名格式：taskId_timestamp.mp4
    // 提取 taskId（第一个下划线之前的部分）
    int underscorePos = fileName.indexOf('_');
    if (underscorePos > 0) {
        return fileName.left(underscorePos);
    }

    // 如果没有下划线，尝试提取 .mp4 之前的部分作为 taskId
    int dotPos = fileName.lastIndexOf('.');
    if (dotPos > 0) {
        return fileName.left(dotPos);
    }

    return "";
}

void MainWindow::onModeChanged(int index) {
    // 0: 文生视频, 1: 图生视频
    bool isImageToVideo = (index == 1);

    // 显示/隐藏图片输入区域
    imageInputWidget->setVisible(isImageToVideo);

    // 更新按钮文本
    if (isImageToVideo) {
        generateBtn->setText("生成视频（图生视频）");
        promptEdit->setPlaceholderText("请输入提示词（可选）...");
    } else {
        generateBtn->setText("生成视频");
        promptEdit->setPlaceholderText("请输入提示词...");
    }

    qDebug() << "Mode changed to:" << (isImageToVideo ? "Image-to-Video" : "Text-to-Video");
}

void MainWindow::onSelectImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择首帧图片",
        QDir::homePath(),
        "图片文件 (*.jpg *.jpeg *.png *.webp *.bmp *.tiff *.gif);;所有文件 (*.*)"
    );

    if (!fileName.isEmpty()) {
        firstImagePath = fileName;
        updateImagePreview(imagePreviewLabel, fileName);
        qDebug() << "Selected first image:" << fileName;
    }
}

void MainWindow::onSelectLastImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择尾帧图片（可选）",
        QDir::homePath(),
        "图片文件 (*.jpg *.jpeg *.png *.webp *.bmp *.tiff *.gif);;所有文件 (*.*)"
    );

    if (!fileName.isEmpty()) {
        lastImagePath = fileName;
        updateImagePreview(lastImagePreviewLabel, fileName);
        qDebug() << "Selected last image:" << fileName;
    }
}

QString MainWindow::imageToBase64(const QString &imagePath) const {
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open image file:" << imagePath;
        return "";
    }

    QByteArray imageData = file.readAll();
    file.close();

    // 获取文件扩展名以确定 MIME 类型
    QString extension = QFileInfo(imagePath).suffix().toLower();
    QString mimeType = "image/jpeg";  // 默认

    if (extension == "png") mimeType = "image/png";
    else if (extension == "webp") mimeType = "image/webp";
    else if (extension == "bmp") mimeType = "image/bmp";
    else if (extension == "tiff" || extension == "tif") mimeType = "image/tiff";
    else if (extension == "gif") mimeType = "image/gif";

    // 返回 Base64 格式：data:image/jpeg;base64,<base64-data>
    QString base64 = "data:" + mimeType + ";base64," + imageData.toBase64();

    qDebug() << "Image converted to base64, size:" << base64.length() << "chars";
    return base64;
}

void MainWindow::updateImagePreview(QLabel *label, const QString &imagePath) {
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        label->setText("无法加载图片");
        return;
    }

    // 缩放图片以适应预览区域
    QPixmap scaledPixmap = pixmap.scaled(
        label->maximumWidth() > 0 ? label->maximumWidth() : 300,
        label->maximumHeight(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    label->setPixmap(scaledPixmap);
}

