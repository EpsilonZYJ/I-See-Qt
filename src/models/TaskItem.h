#ifndef TASKITEM_H
#define TASKITEM_H

#include <QString>
#include <QDateTime>

enum class TaskStatus {
    Pending,      // 等待中
    Processing,   // 处理中
    Completed,    // 已完成
    Failed        // 失败
};

struct TaskItem {
    QString taskId;
    QString prompt;
    QString apiKey;

    // 请求参数
    int width = 1280;
    int height = 720;
    QString resolution = "1080p";
    QString aspectRatio = "16:9";
    int duration = 5;
    bool cameraFixed = false;
    int seed = 123;

    // 任务状态
    TaskStatus status = TaskStatus::Pending;
    QString errorMessage;
    QString videoUrl;
    QString localFilePath;

    // 时间戳
    QDateTime createTime;
    QDateTime updateTime;
    QDateTime completeTime;

    // 辅助方法
    QString statusString() const {
        switch(status) {
            case TaskStatus::Pending: return "等待中";
            case TaskStatus::Processing: return "处理中";
            case TaskStatus::Completed: return "已完成";
            case TaskStatus::Failed: return "失败";
            default: return "未知";
        }
    }

    bool isFinished() const {
        return status == TaskStatus::Completed || status == TaskStatus::Failed;
    }
};

#endif // TASKITEM_H

