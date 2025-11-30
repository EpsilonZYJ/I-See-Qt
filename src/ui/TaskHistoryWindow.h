#ifndef TASKHISTORYWINDOW_H
#define TASKHISTORYWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QTimer>
#include <QTextEdit>
#include <QLabel>
#include "models/TaskItem.h"

class TaskDatabaseService;
class ApiService;

class TaskHistoryWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit TaskHistoryWindow(TaskDatabaseService *dbService, ApiService *apiService, QWidget *parent = nullptr);
    ~TaskHistoryWindow();

    void refreshTasks();

public slots:
    void onTaskPolled(const QString &taskId, bool success, const QString &videoUrl, const QString &error);

signals:
    void taskStatusChanged(const QString &taskId);

private slots:
    void onTableItemSelectionChanged();
    void onRefreshClicked();
    void onDeleteClicked();
    void onAutoRefreshTimeout();
    void onQueryByTaskId();
    void onTableItemDoubleClicked(QTableWidgetItem *item);

private:
    void setupUi();
    void loadTasks();
    void updateTaskRow(int row, const TaskItem &task);
    void showTaskDetails(const TaskItem &task);
    void pollPendingTask(const TaskItem &task);
    void downloadVideoForTask(const QString &taskId, const QString &videoUrl);
    void onVideoDownloadedForTask(const QString &taskId, const QString &localPath);
    void retryFailedTasks();  // 重试失败的任务

    TaskDatabaseService *dbService;
    ApiService *apiService;

    // UI 组件
    QTableWidget *taskTable;
    QPushButton *refreshBtn;
    QPushButton *deleteBtn;
    QPushButton *queryByIdBtn;
    QLineEdit *taskIdInput;
    QLineEdit *apiKeyInput;
    QTextEdit *detailsText;
    QLabel *statusLabel;

    // 自动刷新定时器
    QTimer *autoRefreshTimer;

    QList<TaskItem> currentTasks;
    QString currentSelectedTaskId;
};

#endif // TASKHISTORYWINDOW_H

