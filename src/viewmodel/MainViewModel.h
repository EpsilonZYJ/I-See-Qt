#ifndef MAINVIEWMODEL_H
#define MAINVIEWMODEL_H

#include "const/QtHeaders.h"
#include "services/ApiService.h"
#include "services/HistoryService.h"

class TaskDatabaseService;

class MainViewModel : public QObject {
    Q_OBJECT
public:
    explicit MainViewModel(QObject *parent = nullptr);

    // 给 UI 调用的方法
    void startGeneration(const QString &apiKey, const QString &prompt);
    void loadHistory();
    void deleteHistoryItem(int index);
    QList<HistoryItem> getHistory() const;
    QString getCurrentSavePath() const;

    // 获取数据库服务
    TaskDatabaseService* getTaskDatabaseService() const;

signals:
    // 通知 UI 更新的信号
    void statusChanged(const QString &msg);
    void progressUpdated(int value);
    void videoReady(const QString &localPath); // 新生成的视频
    void historyUpdated(); // 列表变化
    void errorOccurred(const QString &msg);

private slots:
    void onTaskSubmitted(const QString &taskId);
    void onTaskFinished(bool success, const QString &result, const QString &error);
    void onVideoDownloaded(const QString &tempPath);

private:
    ApiService *apiService;
    HistoryService *historyService;
    TaskDatabaseService *taskDbService;
    QTimer *pollTimer;
    QString currentTaskId;
    QString currentApiKey;
    QString currentPrompt;
};

#endif // MAINVIEWMODEL_H