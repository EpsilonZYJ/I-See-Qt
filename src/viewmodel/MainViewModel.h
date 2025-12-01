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
    void startGeneration(const QString &apiKey, const QString &prompt, const QMap<QString, QString> &params = QMap<QString, QString>());
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
    void onSmartPoll();  // 智能轮询槽函数

private:
    void startSmartPolling();  // 开始智能轮询
    void stopPolling();  // 停止轮询
    void updateWaitingTime();  // 更新等待时间显示

    ApiService *apiService;
    HistoryService *historyService;
    TaskDatabaseService *taskDbService;
    QTimer *pollTimer;
    QString currentTaskId;
    QString currentApiKey;
    QString currentPrompt;
    QMap<QString, QString> currentParams;  // 当前任务参数

    // 智能轮询相关
    QDateTime taskStartTime;  // 任务开始时间
    int pollAttempts;  // 轮询次数
    int currentInterval;  // 当前轮询间隔（毫秒）
    static const int MAX_WAIT_TIME = 300;  // 最大等待时间（秒）5分钟
    static const int INITIAL_INTERVAL = 3000;  // 初始轮询间隔3秒
    static const int MAX_INTERVAL = 30000;  // 最大轮询间隔30秒
};

#endif // MAINVIEWMODEL_H