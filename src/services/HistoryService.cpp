#include "HistoryService.h"
#include "const/AppConfig.h"
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>

HistoryService::HistoryService(QObject *parent) : QObject(parent) {}

QString HistoryService::getSavePath() const {
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    return settings.value(Config::KEY_SAVE_PATH).toString();
}

void HistoryService::load() {
    items.clear();
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    QFile file(settings.fileName() + ".json"); // 存放在配置同级目录
    if(file.open(QIODevice::ReadOnly)) {
        QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
        for(const auto &val : arr) {
            QJsonObject obj = val.toObject();
            items.append({obj["prompt"].toString(), obj["path"].toString(), obj["date"].toString()});
        }
    }
}

void HistoryService::saveJson() {
    QJsonArray arr;
    for(const auto &item : items) {
        QJsonObject obj;
        obj["prompt"] = item.prompt;
        obj["path"] = item.filePath;
        obj["date"] = item.date;
        arr.append(obj);
    }
    QSettings settings(Config::ORG_NAME, Config::APP_NAME);
    QFile file(settings.fileName() + ".json");
    if(file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
    }
}

void HistoryService::add(const QString &prompt, const QString &path) {
    items.prepend({prompt, path, QDateTime::currentDateTime().toString("MM-dd HH:mm")});
    saveJson();
}

void HistoryService::remove(int index) {
    if(index >= 0 && index < items.size()) {
        QFile::remove(items[index].filePath); // 删除物理文件
        items.removeAt(index);
        saveJson();
    }
}

QList<HistoryItem> HistoryService::getItems() const {
    return items;
}