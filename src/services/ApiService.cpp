#include "ApiService.h"
#include "const/AppConfig.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

ApiService::ApiService(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
    loadApiUrls();
}

void ApiService::loadApiUrls() {
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);

    // 使用 SUBMIT_URL 和 QUERY_URL 的值作为设置键来读取自定义 URL
    // 如果没有设置，则返回默认值（Config::SUBMIT_URL 和 Config::QUERY_URL）
    submitUrl = settings.value("submitUrl", Config::SUBMIT_URL).toString();
    queryUrl = settings.value("queryUrl", Config::QUERY_URL).toString();

    qDebug() << "API URLs loaded:";
    qDebug() << "  Submit:" << submitUrl << (submitUrl == Config::SUBMIT_URL ? "(default)" : "(custom)");
    qDebug() << "  Query:" << queryUrl << (queryUrl == Config::QUERY_URL ? "(default)" : "(custom)");
}

void ApiService::setSubmitUrl(const QString &url) {
    submitUrl = url;
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    settings.setValue("submitUrl", url);
    qDebug() << "Submit URL saved:" << url;
}

void ApiService::setQueryUrl(const QString &url) {
    queryUrl = url;
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    settings.setValue("queryUrl", url);
    qDebug() << "Query URL saved:" << url;
}

QString ApiService::getSubmitUrl() const {
    return submitUrl;
}

QString ApiService::getQueryUrl() const {
    return queryUrl;
}

void ApiService::reloadApiUrls() {
    loadApiUrls();
    qDebug() << "API URLs reloaded";
}

void ApiService::submitTask(const QString &apiKey, const QString &prompt) {
    QNetworkRequest request{QUrl(submitUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QJsonObject json;
    // json["model"] = "seedance-v1-pro-t2v";
    json["prompt"] = prompt;
    json["width"] = 1280;
    json["resolution"] = "1080p";
    json["aspect_ratio"] = "16:9";
    json["duration"] = 5;  // API 只接受 5 或 10
    json["camera_fixed"] = false;
    json["seed"] = 123;
    // json["height"] = 720;
    // json["video_length"] = 5;
    // json["seed"] = QRandomGenerator::global()->bounded(1000000);

    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    qDebug() << "Submitting JSON:" << QString::fromUtf8(jsonData);

    QNetworkReply *reply = manager->post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        reply->deleteLater();
        if (reply->error()) {
            QByteArray responseData = reply->readAll();
            QString errorDetails = QString::fromUtf8(responseData);
            qDebug() << "HTTP Error:" << reply->error() << reply->errorString();
            qDebug() << "Server Response:" << errorDetails;
            emit errorOccurred("提交失败: " + reply->errorString() + "\n详情: " + errorDetails);
            return;
        }
        QByteArray responseData = reply->readAll();
        qDebug() << "Success Response:" << QString::fromUtf8(responseData);
        QJsonObject resp = QJsonDocument::fromJson(responseData).object();
        QString taskId;
        if(resp.contains("task_id")) taskId = resp["task_id"].toString();
        else if(resp.contains("id")) taskId = resp["id"].toString();
        else if(resp.contains("data")) taskId = resp["data"].toObject()["id"].toString();

        if(!taskId.isEmpty()) emit taskSubmitted(taskId);
        else emit errorOccurred("未获取到 Task ID");
    });
}

void ApiService::pollTask(const QString &apiKey, const QString &taskId) {
    // 使用查询参数而不是路径参数
    QString urlStr = queryUrl + "?task_id=" + taskId;

    QNetworkRequest request{QUrl(urlStr)};
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        reply->deleteLater();
        if(reply->error()) {
            emit errorOccurred("轮询失败: " + reply->errorString());
            emit taskPolled(taskId, false, "", reply->errorString());
            return;
        }

        QByteArray responseData = reply->readAll();
        qDebug() << "Poll response for" << taskId << ":" << QString::fromUtf8(responseData);
        QJsonObject resp = QJsonDocument::fromJson(responseData).object();

        // 解析 task 对象
        QJsonObject taskObj = resp["task"].toObject();
        QString status = taskObj["status"].toString();

        qDebug() << "Task status:" << status;

        if (status == "TASK_STATUS_SUCCEED") {
            // 从 videos 数组中获取视频 URL
            QString videoUrl;
            QJsonArray videos = resp["videos"].toArray();
            if (!videos.isEmpty()) {
                QJsonObject videoObj = videos[0].toObject();
                videoUrl = videoObj["video_url"].toString();
            }

            qDebug() << "Task succeeded, video URL:" << videoUrl;
            emit taskFinished(true, videoUrl, "");
            emit taskPolled(taskId, true, videoUrl, "");
        } else if (status == "TASK_STATUS_FAILED") {
            QString error = taskObj["reason"].toString();
            if (error.isEmpty()) {
                error = "任务失败";
            }
            qDebug() << "Task failed:" << error;
            emit taskFinished(false, "", error);
            emit taskPolled(taskId, false, "", error);
        } else if (status == "TASK_STATUS_PROCESSING" || status == "TASK_STATUS_QUEUED") {
            // 仍在处理中或排队中
            int progressPercent = taskObj["progress_percent"].toInt();
            qDebug() << "Task processing, progress:" << progressPercent << "%";
            emit errorOccurred("STATUS_PROCESSING"); // 用一个特殊字��通知 VM 继续
            emit taskPolled(taskId, false, "", "STATUS_PROCESSING");
        } else {
            // 未知状态
            qDebug() << "Unknown task status:" << status;
            emit taskPolled(taskId, false, "", "未知状态: " + status);
        }
    });
}

void ApiService::pollAllTasks(const QString &apiKey) {
    // 批量查询所有任务，不需要 task_id 参数
    QString urlStr = queryUrl;

    QNetworkRequest request{QUrl(urlStr)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    qDebug();
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        reply->deleteLater();
        if(reply->error()) {
            qDebug() << "Batch poll failed:" << reply->errorString();
            emit errorOccurred("批量查询失败: " + reply->errorString());
            return;
        }

        QByteArray responseData = reply->readAll();
        qDebug() << "Batch poll response:" << QString::fromUtf8(responseData);

        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject resp = doc.object();

        // 发射批量查询结果信号
        emit allTasksPolled(resp);

        // 解析 task 对象（如果返回单个任务）
        if (resp.contains("task")) {
            QJsonObject taskObj = resp["task"].toObject();
            QString taskId = taskObj["task_id"].toString();
            QString status = taskObj["status"].toString();

            qDebug() << "Batch response contains task:" << taskId << "status:" << status;

            if (status == "TASK_STATUS_SUCCEED") {
                QString videoUrl;
                QJsonArray videos = resp["videos"].toArray();
                if (!videos.isEmpty()) {
                    QJsonObject videoObj = videos[0].toObject();
                    videoUrl = videoObj["video_url"].toString();
                }
                emit taskPolled(taskId, true, videoUrl, "");
            } else if (status == "TASK_STATUS_FAILED") {
                QString error = taskObj["reason"].toString();
                emit taskPolled(taskId, false, "", error);
            } else {
                emit taskPolled(taskId, false, "", "STATUS_PROCESSING");
            }
        }
    });
}

void ApiService::downloadVideo(const QString &url) {
    QNetworkRequest request{QUrl(url)};
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [=, this]() {
        reply->deleteLater();
        if(reply->error()) {
            emit errorOccurred("下载失败");
            return;
        }
        // 临时保存到缓存目录，实际保存逻辑由 ViewModel/HistoryService 处理
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/temp_video.mp4";
        QFile file(tempPath);
        if(file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            emit videoDownloaded(tempPath);
        }
    });
}