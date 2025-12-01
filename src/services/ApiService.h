#ifndef APISERVICE_H
#define APISERVICE_H

#include "const/QtHeaders.h"

class ApiService : public QObject {
    Q_OBJECT
public:
    explicit ApiService(QObject *parent = nullptr);
    void submitTask(const QString &apiKey, const QString &prompt);
    void pollTask(const QString &apiKey, const QString &taskId);
    void pollAllTasks(const QString &apiKey);  // 新增：批量查询所有任务
    void downloadVideo(const QString &url);

    // API 端点配置
    void setSubmitUrl(const QString &url);
    void setQueryUrl(const QString &url);
    QString getSubmitUrl() const;
    QString getQueryUrl() const;
    void reloadApiUrls();  // 重新加载 API URLs

signals:
    void taskSubmitted(const QString &taskId);
    void taskFinished(bool success, const QString &result, const QString &errorMsg); // result is URL if success
    void taskPolled(const QString &taskId, bool success, const QString &videoUrl, const QString &error); // 任务轮询结果
    void allTasksPolled(const QJsonObject &response); // 新增：批量查询结果
    void videoDownloaded(const QString &localPath);
    void errorOccurred(const QString &msg);

private:
    QNetworkAccessManager *manager;
    QString submitUrl;  // 提交任务的 URL
    QString queryUrl;   // 查询任务的 URL

    void loadApiUrls();  // 从设置加载 API URL
};

#endif // APISERVICE_H