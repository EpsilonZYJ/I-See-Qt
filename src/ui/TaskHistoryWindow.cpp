#include "TaskHistoryWindow.h"
#include "services/TaskDatabaseService.h"
#include "services/ApiService.h"
#include "const/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QGroupBox>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>

TaskHistoryWindow::TaskHistoryWindow(TaskDatabaseService *dbService, ApiService *apiService, QWidget *parent)
    : QMainWindow(parent), dbService(dbService), apiService(apiService) {

    setWindowTitle("任务历史查询");
    resize(1200, 700);

    setupUi();

    // 从配置中加载默认 API Key
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    QString savedApiKey = settings.value(Config::KEY_API_TOKEN).toString();
    if (!savedApiKey.isEmpty()) {
        apiKeyInput->setText(savedApiKey);
    }

    loadTasks();

    // 自动重试查询失败的任务
    retryFailedTasks();

    // 自动刷新未完成的任务（每30秒）
    autoRefreshTimer = new QTimer(this);
    connect(autoRefreshTimer, &QTimer::timeout, this, &TaskHistoryWindow::onAutoRefreshTimeout);
    autoRefreshTimer->start(30000);  // 30秒
}

TaskHistoryWindow::~TaskHistoryWindow() {
}

void TaskHistoryWindow::setupUi() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // 顶部工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();

    // Task ID 查询区域
    QLabel *taskIdLabel = new QLabel("Task ID:", this);
    taskIdInput = new QLineEdit(this);
    taskIdInput->setPlaceholderText("输入 Task ID 或留空查询所有任务...");
    taskIdInput->setMinimumWidth(250);

    QLabel *apiKeyLabel = new QLabel("API Key:", this);
    apiKeyInput = new QLineEdit(this);
    apiKeyInput->setPlaceholderText("输入 API Key...");
    apiKeyInput->setEchoMode(QLineEdit::Password);
    apiKeyInput->setMinimumWidth(150);

    queryByIdBtn = new QPushButton("查询", this);
    queryByIdBtn->setToolTip("根据 Task ID 查询任务");

    toolbarLayout->addWidget(taskIdLabel);
    toolbarLayout->addWidget(taskIdInput);
    toolbarLayout->addWidget(apiKeyLabel);
    toolbarLayout->addWidget(apiKeyInput);
    toolbarLayout->addWidget(queryByIdBtn);
    toolbarLayout->addWidget(new QWidget(this), 1); // 分隔符

    refreshBtn = new QPushButton("刷新", this);
    deleteBtn = new QPushButton("删除选中", this);
    deleteBtn->setEnabled(false);
    statusLabel = new QLabel("", this);

    toolbarLayout->addWidget(refreshBtn);
    toolbarLayout->addWidget(deleteBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(statusLabel);

    mainLayout->addLayout(toolbarLayout);

    // 使用分割器布局任务列表和详情
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // 任务列表表格
    taskTable = new QTableWidget(this);
    taskTable->setColumnCount(6);
    taskTable->setHorizontalHeaderLabels({"任务ID", "提示词", "状态", "创建时间", "完成时间", "视频URL"});
    taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    taskTable->setSelectionMode(QAbstractItemView::SingleSelection);
    taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    taskTable->horizontalHeader()->setStretchLastSection(true);
    taskTable->setColumnWidth(0, 200);
    taskTable->setColumnWidth(1, 250);
    taskTable->setColumnWidth(2, 80);
    taskTable->setColumnWidth(3, 150);
    taskTable->setColumnWidth(4, 150);

    splitter->addWidget(taskTable);

    // 详情面板
    QGroupBox *detailsBox = new QGroupBox("任务详情", this);
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsBox);
    detailsText = new QTextEdit(this);
    detailsText->setReadOnly(true);
    detailsLayout->addWidget(detailsText);

    splitter->addWidget(detailsBox);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter);

    // 连接信号
    connect(taskTable, &QTableWidget::itemSelectionChanged, this, &TaskHistoryWindow::onTableItemSelectionChanged);
    connect(taskTable, &QTableWidget::itemDoubleClicked, this, &TaskHistoryWindow::onTableItemDoubleClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &TaskHistoryWindow::onRefreshClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &TaskHistoryWindow::onDeleteClicked);
    connect(queryByIdBtn, &QPushButton::clicked, this, &TaskHistoryWindow::onQueryByTaskId);
}

void TaskHistoryWindow::loadTasks() {
    currentTasks = dbService->getAllTasks();

    taskTable->setRowCount(0);

    for (int i = 0; i < currentTasks.size(); ++i) {
        const TaskItem &task = currentTasks[i];
        taskTable->insertRow(i);
        updateTaskRow(i, task);
    }

    statusLabel->setText(QString("共 %1 个任务").arg(currentTasks.size()));
}

void TaskHistoryWindow::updateTaskRow(int row, const TaskItem &task) {
    taskTable->setItem(row, 0, new QTableWidgetItem(task.taskId));

    // 截断过长的提示词
    QString promptPreview = task.prompt;
    if (promptPreview.length() > 30) {
        promptPreview = promptPreview.left(30) + "...";
    }
    taskTable->setItem(row, 1, new QTableWidgetItem(promptPreview));

    // 状态带颜色
    QTableWidgetItem *statusItem = new QTableWidgetItem(task.statusString());
    switch(task.status) {
        case TaskStatus::Completed:
            statusItem->setForeground(Qt::darkGreen);
            break;
        case TaskStatus::Failed:
            statusItem->setForeground(Qt::red);
            break;
        case TaskStatus::Processing:
            statusItem->setForeground(Qt::blue);
            break;
        default:
            statusItem->setForeground(Qt::gray);
    }
    taskTable->setItem(row, 2, statusItem);

    taskTable->setItem(row, 3, new QTableWidgetItem(task.createTime.toString("yyyy-MM-dd HH:mm:ss")));

    QString completeTimeStr = task.completeTime.isValid() ?
        task.completeTime.toString("yyyy-MM-dd HH:mm:ss") : "-";
    taskTable->setItem(row, 4, new QTableWidgetItem(completeTimeStr));

    QString videoUrlPreview = task.videoUrl.isEmpty() ? "-" :
        (task.videoUrl.length() > 40 ? task.videoUrl.left(40) + "..." : task.videoUrl);
    taskTable->setItem(row, 5, new QTableWidgetItem(videoUrlPreview));
}

void TaskHistoryWindow::showTaskDetails(const TaskItem &task) {
    QString details;
    details += "========== 任务详情 ==========\n\n";
    details += "任务ID: " + task.taskId + "\n\n";

    // 提示词（如果为空说明是通过 task_id 查询的）
    if (task.prompt.isEmpty()) {
        details += "提示词: (通过 Task ID 查询，无原始提示词)\n\n";
    } else {
        details += "提示词:\n" + task.prompt + "\n\n";
    }

    details += "---------- 参数信息 ----------\n";

    // 判断是否为通过 task_id 查询的任务（参数为空）
    if (task.resolution.isEmpty() && task.width == 0 && task.height == 0) {
        details += "(通过 Task ID 查询，无原始请求参数)\n\n";
    } else {
        details += QString("分辨率: %1 (%2)\n").arg(task.resolution).arg(task.aspectRatio);
        details += QString("尺寸: %1 x %2\n").arg(task.width).arg(task.height);
        details += QString("时长: %1 秒\n").arg(task.duration);
        details += QString("固定相机: %1\n").arg(task.cameraFixed ? "是" : "否");
        details += QString("随机种子: %1\n\n").arg(task.seed);
    }

    details += "---------- 状态信息 ----------\n";
    details += "状态: " + task.statusString() + "\n";

    if (task.prompt.isEmpty()) {
        details += "查询时间: " + task.createTime.toString("yyyy-MM-dd HH:mm:ss") + "\n";
    } else {
        details += "创建时间: " + task.createTime.toString("yyyy-MM-dd HH:mm:ss") + "\n";
    }

    if (task.updateTime.isValid()) {
        details += "更新时间: " + task.updateTime.toString("yyyy-MM-dd HH:mm:ss") + "\n";
    }

    if (task.completeTime.isValid()) {
        details += "完成时间: " + task.completeTime.toString("yyyy-MM-dd HH:mm:ss") + "\n";
        int elapsedSeconds = task.createTime.secsTo(task.completeTime);
        details += QString("耗时: %1 分 %2 秒\n").arg(elapsedSeconds / 60).arg(elapsedSeconds % 60);
    }

    if (!task.errorMessage.isEmpty()) {
        details += "\n---------- 错误信息 ----------\n";
        details += task.errorMessage + "\n";
    }

    if (!task.videoUrl.isEmpty()) {
        details += "\n---------- 视频信息 ----------\n";
        details += "视频URL: " + task.videoUrl + "\n";
    }

    if (!task.localFilePath.isEmpty()) {
        details += "本地路径: " + task.localFilePath + "\n";
    }

    detailsText->setText(details);
}

void TaskHistoryWindow::onTableItemSelectionChanged() {
    QList<QTableWidgetItem*> selected = taskTable->selectedItems();
    if (!selected.isEmpty()) {
        int row = taskTable->row(selected.first());
        if (row >= 0 && row < currentTasks.size()) {
            currentSelectedTaskId = currentTasks[row].taskId;
            showTaskDetails(currentTasks[row]);
            deleteBtn->setEnabled(true);
        }
    } else {
        currentSelectedTaskId.clear();
        detailsText->clear();
        deleteBtn->setEnabled(false);
    }
}

void TaskHistoryWindow::onTableItemDoubleClicked(QTableWidgetItem *item) {
    if (!item) {
        return;
    }

    int row = taskTable->row(item);
    if (row < 0 || row >= currentTasks.size()) {
        return;
    }

    const TaskItem &task = currentTasks[row];

    // 检查本地文件路径
    if (task.localFilePath.isEmpty()) {
        QMessageBox::information(this, "提示",
            "该任务还没有本地视频文件。\n\n"
            "视频URL: " + task.videoUrl);
        return;
    }

    // 检查文件是否存在
    QFileInfo fileInfo(task.localFilePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "文件不存在",
            "本地视频文件不存在：\n" + task.localFilePath + "\n\n"
            "可能已被删除或移动。");
        return;
    }

    // 使用系统默认播放器打开视频
    QUrl fileUrl = QUrl::fromLocalFile(task.localFilePath);
    bool success = QDesktopServices::openUrl(fileUrl);

    if (!success) {
        QMessageBox::warning(this, "打开失败",
            "无法使用系统默认播放器打开视频。\n\n"
            "文件路径: " + task.localFilePath);
    } else {
        qDebug() << "Opening video with system player:" << task.localFilePath;
        statusLabel->setText("已打开视频: " + task.taskId);
    }
}

void TaskHistoryWindow::onRefreshClicked() {
    refreshTasks();
}

void TaskHistoryWindow::refreshTasks() {
    statusLabel->setText("刷新中...");

    // 重新加载所有任务
    loadTasks();

    // 轮询未完成的任务
    QList<TaskItem> pendingTasks = dbService->getPendingTasks();
    if (!pendingTasks.isEmpty()) {
        statusLabel->setText(QString("正在轮询 %1 个未完成任务...").arg(pendingTasks.size()));
        for (const TaskItem &task : pendingTasks) {
            pollPendingTask(task);
        }
    } else {
        statusLabel->setText(QString("共 %1 个任务，无待处理任务").arg(currentTasks.size()));
    }
}

void TaskHistoryWindow::onDeleteClicked() {
    if (currentSelectedTaskId.isEmpty()) {
        return;
    }

    auto reply = QMessageBox::question(this, "确认删除",
        "确定要删除选中的任务吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (dbService->deleteTask(currentSelectedTaskId)) {
            QMessageBox::information(this, "成功", "任务已删除");
            loadTasks();
        } else {
            QMessageBox::warning(this, "错误", "删除任务失败");
        }
    }
}

void TaskHistoryWindow::onAutoRefreshTimeout() {
    // 自动刷新未完成的任务
    QList<TaskItem> pendingTasks = dbService->getPendingTasks();
    if (!pendingTasks.isEmpty()) {
        qDebug() << "Auto refreshing" << pendingTasks.size() << "pending tasks";
        for (const TaskItem &task : pendingTasks) {
            pollPendingTask(task);
        }
    }
}

void TaskHistoryWindow::pollPendingTask(const TaskItem &task) {
    // 连接 ApiService 的轮询信号（需要修改 ApiService）
    // 这里暂时使用简化方式
    apiService->pollTask(task.apiKey, task.taskId);
}

void TaskHistoryWindow::onTaskPolled(const QString &taskId, bool success, const QString &videoUrl, const QString &error) {
    TaskItem task = dbService->getTask(taskId);
    if (task.taskId.isEmpty()) {
        return;
    }

    task.updateTime = QDateTime::currentDateTime();

    if (success) {
        task.status = TaskStatus::Completed;
        task.videoUrl = videoUrl;
        task.completeTime = QDateTime::currentDateTime();

        // 如果有视频 URL 但没有本地文件，自动下载
        if (!videoUrl.isEmpty() && task.localFilePath.isEmpty()) {
            qDebug() << "Auto downloading video for task:" << taskId;
            downloadVideoForTask(taskId, videoUrl);
        }
    } else if (!error.isEmpty() && error != "STATUS_PROCESSING") {
        task.status = TaskStatus::Failed;
        task.errorMessage = error;
        task.completeTime = QDateTime::currentDateTime();
    } else {
        // 仍在处理中
        task.status = TaskStatus::Processing;
    }

    dbService->updateTask(task);

    // 刷新显示
    loadTasks();

    emit taskStatusChanged(taskId);
}

void TaskHistoryWindow::onQueryByTaskId() {
    QString taskId = taskIdInput->text().trimmed();
    QString apiKey = apiKeyInput->text().trimmed();

    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入 API Key");
        return;
    }

    // 如果 Task ID 为空，使用批量查询
    if (taskId.isEmpty()) {
        statusLabel->setText("正在批量查询所有任务...");
        queryByIdBtn->setEnabled(false);

        // 调用批量查询 API
        apiService->pollAllTasks(apiKey);

        // 延迟恢复按钮
        QTimer::singleShot(3000, this, [this]() {
            queryByIdBtn->setEnabled(true);
            statusLabel->setText("批量查询完成");
        });

        return;
    }

    // 如果有 Task ID，查询单个任务
    // 检查是否已存在
    TaskItem existingTask = dbService->getTask(taskId);
    if (!existingTask.taskId.isEmpty()) {
        auto reply = QMessageBox::question(this, "任务已存在",
            "该 Task ID 已存在于数据库中，是否重新查询更新状态？",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    statusLabel->setText("正在查询 Task ID: " + taskId);
    queryByIdBtn->setEnabled(false);

    // 调用 API 查询任务状态
    apiService->pollTask(apiKey, taskId);

    // 监听查询结果
    QTimer::singleShot(5000, this, [this, taskId, apiKey]() {
        TaskItem task = dbService->getTask(taskId);

        // 如果是新任务（不存在），创建一个新记录
        if (task.taskId.isEmpty()) {
            task.taskId = taskId;
            task.prompt = "";  // 通过 task_id 查询的，prompt 为空
            task.apiKey = apiKey;
            task.status = TaskStatus::Pending;
            task.createTime = QDateTime::currentDateTime();  // 使用查询时间作为创建时间
            task.updateTime = task.createTime;

            // 参数设置为空/默认值
            task.width = 0;
            task.height = 0;
            task.resolution = "";
            task.aspectRatio = "";
            task.duration = 0;
            task.cameraFixed = false;
            task.seed = 0;

            dbService->saveTask(task);
        }

        loadTasks();
        statusLabel->setText("查询完成");
        queryByIdBtn->setEnabled(true);
    });
}

void TaskHistoryWindow::downloadVideoForTask(const QString &taskId, const QString &videoUrl) {
    if (videoUrl.isEmpty()) {
        qDebug() << "Video URL is empty for task:" << taskId;
        return;
    }

    // 创建保存目录
    QString saveDir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/I See";
    QDir dir;
    if (!dir.exists(saveDir)) {
        dir.mkpath(saveDir);
    }

    // 生成文件名：taskId_timestamp.mp4
    QString fileName = taskId + "_" + QString::number(QDateTime::currentSecsSinceEpoch()) + ".mp4";
    QString localPath = saveDir + "/" + fileName;

    qDebug() << "Downloading video from:" << videoUrl;
    qDebug() << "Saving to:" << localPath;

    // 创建 QNetworkAccessManager 进行下载
    QNetworkAccessManager *downloadManager = new QNetworkAccessManager(this);
    QNetworkRequest request{QUrl(videoUrl)};
    QNetworkReply *reply = downloadManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        reply->deleteLater();
        downloadManager->deleteLater();

        if (reply->error()) {
            qDebug() << "Download failed for task" << taskId << ":" << reply->errorString();
            statusLabel->setText("下载失败: " + taskId);
            return;
        }

        // 保存文件
        QFile file(localPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();

            qDebug() << "Video downloaded successfully to:" << localPath;
            onVideoDownloadedForTask(taskId, localPath);
            statusLabel->setText("视频已下载: " + taskId);
        } else {
            qDebug() << "Failed to open file for writing:" << localPath;
            statusLabel->setText("保存失败: " + taskId);
        }
    });

    // 监听下载进度
    connect(reply, &QNetworkReply::downloadProgress, this, [=](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            int percent = (bytesReceived * 100) / bytesTotal;
            statusLabel->setText(QString("下载中 %1: %2%").arg(taskId).arg(percent));
        }
    });
}

void TaskHistoryWindow::onVideoDownloadedForTask(const QString &taskId, const QString &localPath) {
    // 更新数据库中的本地文件路径
    TaskItem task = dbService->getTask(taskId);
    if (!task.taskId.isEmpty()) {
        task.localFilePath = localPath;
        task.updateTime = QDateTime::currentDateTime();
        dbService->updateTask(task);

        qDebug() << "Updated task" << taskId << "with local path:" << localPath;

        // 刷新显示
        loadTasks();

        // 如果当前选中的是这个任务，刷新详情
        if (currentSelectedTaskId == taskId) {
            showTaskDetails(task);
        }
    }
}

void TaskHistoryWindow::retryFailedTasks() {
    // 获取所有失败的任务（包括超时的）
    QList<TaskItem> allTasks = dbService->getAllTasks();
    QList<TaskItem> failedTasks;

    for (const TaskItem &task : allTasks) {
        if (task.status == TaskStatus::Failed) {
            // 检查是否是查询超时导致的失败
            if (task.errorMessage.contains("超时") || task.errorMessage.contains("timeout")) {
                failedTasks.append(task);
            }
        }
    }

    if (failedTasks.isEmpty()) {
        qDebug() << "No failed tasks to retry";
        return;
    }

    qDebug() << "Found" << failedTasks.size() << "failed tasks, retrying...";
    statusLabel->setText(QString("正在重试 %1 个失败任务...").arg(failedTasks.size()));

    // 对每个失败的任务重新查询
    for (const TaskItem &task : failedTasks) {
        if (!task.apiKey.isEmpty()) {
            qDebug() << "Retrying task:" << task.taskId;

            // 更新状态为处理中
            TaskItem updatedTask = task;
            updatedTask.status = TaskStatus::Processing;
            updatedTask.errorMessage = ""; // 清除错误信息
            updatedTask.updateTime = QDateTime::currentDateTime();
            dbService->updateTask(updatedTask);

            // 发起查询
            apiService->pollTask(task.apiKey, task.taskId);
        }
    }

    // 3秒后刷新列表
    QTimer::singleShot(3000, this, [this]() {
        loadTasks();
        statusLabel->setText("重试完成");
    });
}
