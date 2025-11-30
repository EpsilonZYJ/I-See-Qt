#include "MainViewModel.h"
#include "services/TaskDatabaseService.h"
#include "models/TaskItem.h"

MainViewModel::MainViewModel(QObject *parent) : QObject(parent) {
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
        pollTimer->stop();
    });

    connect(pollTimer, &QTimer::timeout, this, [this](){
        apiService->pollTask(currentApiKey, currentTaskId);
    });
}

void MainViewModel::loadHistory() {
    historyService->load();
    emit historyUpdated();
}

void MainViewModel::startGeneration(const QString &apiKey, const QString &prompt) {
    currentApiKey = apiKey;
    currentPrompt = prompt;
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
    task.width = 1280;
    task.height = 720;
    task.resolution = "1080p";
    task.aspectRatio = "16:9";
    task.duration = 5;
    task.cameraFixed = false;
    task.seed = 123;
    task.status = TaskStatus::Processing;
    task.createTime = QDateTime::currentDateTime();
    task.updateTime = task.createTime;

    taskDbService->saveTask(task);

    emit statusChanged("任务已提交 (" + taskId + ")，正在生成...");
    emit progressUpdated(30);
    pollTimer->start(3000); // 3秒轮询
}

void MainViewModel::onTaskFinished(bool success, const QString &result, const QString &error) {
    if(success) {
        pollTimer->stop();

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
        pollTimer->stop();

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
