#ifndef TASKDATABASESERVICE_H
#define TASKDATABASESERVICE_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "models/TaskItem.h"

class TaskDatabaseService : public QObject {
    Q_OBJECT

public:
    explicit TaskDatabaseService(QObject *parent = nullptr);
    ~TaskDatabaseService();

    // 初始化数据库
    bool initialize();

    // 任务操作
    bool saveTask(const TaskItem &task);
    bool updateTask(const TaskItem &task);
    TaskItem getTask(const QString &taskId);
    QList<TaskItem> getAllTasks();
    QList<TaskItem> getPendingTasks();  // 获取未完成的任务
    bool deleteTask(const QString &taskId);

signals:
    void databaseError(const QString &error);

private:
    QSqlDatabase db;

    bool createTables();
    TaskItem taskFromQuery(class QSqlQuery &query);
};

#endif // TASKDATABASESERVICE_H

