#include "MainViewModel.h"
#include "services/TaskDatabaseService.h"
#include "models/TaskItem.h"

MainViewModel::MainViewModel(QObject *parent) : QObject(parent),
    pollAttempts(0), currentInterval(INITIAL_INTERVAL) {
    apiService = new ApiService(this);
    historyService = new HistoryService(this);
    taskDbService = new TaskDatabaseService(this);
    pollTimer = new QTimer(this);

    // 初始化数据库
    if (!taskDbService->initialize()) {
        qWarning() << "Failed to initialize task database";
    }

    connect(apiService, &ApiService::taskSubmitted, this, &MainViewModel::onTaskSubmitted);
    connect(apiService, &ApiService::taskFinished, this, &MainViewModel::onTaskFinished);
    connect(apiService, &ApiService::videoDownloaded, this, &MainViewModel::onVideoDownloaded);
    connect(apiService, &ApiService::errorOccurred, this, [this](const QString &msg){
        if(msg == "STATUS_PROCESSING") return; // 忽略处理中的内部信号
        emit errorOccurred(msg);
        stopPolling();
    });

    // 使用智能轮询
    connect(pollTimer, &QTimer::timeout, this, &MainViewModel::onSmartPoll);
}

void MainViewModel::loadHistory() {
    historyService->load();
    emit historyUpdated();
}

void MainViewModel::startGeneration(const QString &apiKey, const QString &prompt, const QMap<QString, QString> &params) {
    currentApiKey = apiKey;
    currentPrompt = prompt;
    currentParams = params;  // 保存参数
    emit statusChanged("正在提交任务...");
    emit progressUpdated(10);
    apiService->submitTask(apiKey, prompt);
}

void MainViewModel::onTaskSubmitted(const QString &taskId) {
    currentTaskId = taskId;

    // 保存任务到数据库
    TaskItem task;
    task.taskId = taskId;
    task.prompt = currentPrompt;
    task.apiKey = currentApiKey;

    // 使用用户配置的参数，如果没有则使用默认值
    task.width = currentParams.value("width", "1280").toInt();
    task.height = currentParams.value("height", "720").toInt();
    task.resolution = currentParams.value("resolution", "1080p");
    task.aspectRatio = currentParams.value("aspect_ratio", "16:9");
    task.duration = currentParams.value("duration", "5").toInt();
    task.cameraFixed = (currentParams.value("camera_fixed", "false") == "true");
    task.seed = currentParams.value("seed", "123").toInt();

    task.status = TaskStatus::Processing;
    task.createTime = QDateTime::currentDateTime();
    task.updateTime = task.createTime;

    taskDbService->saveTask(task);

    emit statusChanged("任务已提交 (" + taskId + ")，正在生成...");
    emit progressUpdated(30);

    // 启动智能轮询
    startSmartPolling();
}

void MainViewModel::onTaskFinished(bool success, const QString &result, const QString &error) {
    if(success) {
        stopPolling();

        // 更新数据库中的任务状态
        TaskItem task = taskDbService->getTask(currentTaskId);
        if (!task.taskId.isEmpty()) {
            task.status = TaskStatus::Completed;
            task.videoUrl = result;
            task.updateTime = QDateTime::currentDateTime();
            task.completeTime = task.updateTime;
            taskDbService->updateTask(task);
        }

        emit statusChanged("生成成功，正在下载...");
        emit progressUpdated(80);
        apiService->downloadVideo(result); // result is URL
    } else if (!error.isEmpty()) {
        stopPolling();

        // 更新数据库中的任务状态为失败
        TaskItem task = taskDbService->getTask(currentTaskId);
        if (!task.taskId.isEmpty()) {
            task.status = TaskStatus::Failed;
            task.errorMessage = error;
            task.updateTime = QDateTime::currentDateTime();
            task.completeTime = task.updateTime;
            taskDbService->updateTask(task);
        }

        emit errorOccurred("生成失败: " + error);
        emit progressUpdated(0);
    }
    // 如果还没完成，Timer 会继续触发，这里不用处理
}

void MainViewModel::onVideoDownloaded(const QString &tempPath) {
    // 将临时文件移动到最终保存目录
    QString saveDir = historyService->getSavePath();
    QDir dir(saveDir);
    if (!dir.exists()) dir.mkpath(".");

    QString fileName = "video_" + QString::number(QDateTime::currentSecsSinceEpoch()) + ".mp4";
    QString finalPath = dir.filePath(fileName);

    QFile::remove(finalPath); // 覆盖旧的
    if (QFile::copy(tempPath, finalPath)) {
        QFile::remove(tempPath); // 删掉临时文件
        
        // 更新数据库中的本地文件路径
        TaskItem task = taskDbService->getTask(currentTaskId);
        if (!task.taskId.isEmpty()) {
            task.localFilePath = finalPath;
            task.updateTime = QDateTime::currentDateTime();
            taskDbService->updateTask(task);
        }

        historyService->add(currentPrompt, finalPath);
        emit historyUpdated();
        emit videoReady(finalPath);
        emit statusChanged("完成");
        emit progressUpdated(100);
    } else {
        emit errorOccurred("无法保存视频文件");
    }
}

void MainViewModel::deleteHistoryItem(int index) {
    historyService->remove(index);
    emit historyUpdated();
}

QList<HistoryItem> MainViewModel::getHistory() const {
    return historyService->getItems();
}

QString MainViewModel::getCurrentSavePath() const {
    return historyService->getSavePath();
}

TaskDatabaseService* MainViewModel::getTaskDatabaseService() const {
    return taskDbService;
}

ApiService* MainViewModel::getApiService() const {
    return apiService;
}

void MainViewModel::startSmartPolling() {
    taskStartTime = QDateTime::currentDateTime();
    pollAttempts = 0;
    currentInterval = INITIAL_INTERVAL;

    // 立即进行第一次查询
    onSmartPoll();
}

void MainViewModel::stopPolling() {
    pollTimer->stop();
    pollAttempts = 0;
    currentInterval = INITIAL_INTERVAL;
}

void MainViewModel::onSmartPoll() {
    // 检查是否超时（5分钟）
    int elapsedSeconds = taskStartTime.secsTo(QDateTime::currentDateTime());

    if (elapsedSeconds > MAX_WAIT_TIME) {
        // 超时，标记为失败并保存
        stopPolling();

        TaskItem task = taskDbService->getTask(currentTaskId);
        if (!task.taskId.isEmpty()) {
            task.status = TaskStatus::Failed;
            task.errorMessage = "查询超时（超过5分钟）";
            task.updateTime = QDateTime::currentDateTime();
            taskDbService->updateTask(task);
        }

        emit statusChanged("查询超时");
        emit progressUpdated(0);
        emit errorOccurred("任务查询超时（超过5分钟）\n"
                          "任务 ID: " + currentTaskId + "\n"
                          "任务已保存到历史记录，可在任务历史窗口重新查询");
        return;
    }

    // 更新等待时间显示
    updateWaitingTime();

    // 执行查询
    pollAttempts++;
    qDebug() << "Smart poll attempt" << pollAttempts << "after" << elapsedSeconds << "seconds, interval:" << currentInterval << "ms";
    apiService->pollTask(currentApiKey, currentTaskId);

    // 计算下一次查询间隔（指数退避）
    // 间隔序列：3s, 5s, 8s, 13s, 21s, 30s(max)
    int nextInterval = currentInterval;
    if (pollAttempts >= 3) {
        // 从第3次开始使用指数退避
        nextInterval = qMin(currentInterval * 1.6, (double)MAX_INTERVAL);
    }
    currentInterval = nextInterval;

    // 设置下一次查询
    pollTimer->start(currentInterval);
}

void MainViewModel::updateWaitingTime() {
    int elapsedSeconds = taskStartTime.secsTo(QDateTime::currentDateTime());
    int remainingSeconds = MAX_WAIT_TIME - elapsedSeconds;

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    QString timeStr = QString("已等待: %1分%2秒").arg(minutes).arg(seconds, 2, 10, QChar('0'));

    if (remainingSeconds < 60) {
        timeStr += QString(" (剩余: %1秒)").arg(remainingSeconds);
    }

    emit statusChanged("正在生成视频... " + timeStr);

    // 更新进度条（基于时间）
    int progress = 30 + (elapsedSeconds * 50 / MAX_WAIT_TIME); // 30-80%
    progress = qMin(progress, 75); // 最多75%
    emit progressUpdated(progress);
}

