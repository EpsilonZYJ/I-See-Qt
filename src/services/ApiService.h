#ifndef APISERVICE_H
#define APISERVICE_H

#include "const/QtHeaders.h"

class ApiService : public QObject {
    Q_OBJECT
public:
    explicit ApiService(QObject *parent = nullptr);
    void submitTask(const QString &apiKey, const QString &prompt);
    void pollTask(const QString &apiKey, const QString &taskId);
    void downloadVideo(const QString &url);

signals:
    void taskSubmitted(const QString &taskId);
    void taskFinished(bool success, const QString &result, const QString &errorMsg); // result is URL if success
    void taskPolled(const QString &taskId, bool success, const QString &videoUrl, const QString &error); // 新增：任务轮询结果
    void videoDownloaded(const QString &localPath);
    void errorOccurred(const QString &msg);

private:
    QNetworkAccessManager *manager;
};

#endif // APISERVICE_H