#ifndef HISTORYSERVICE_H
#define HISTORYSERVICE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>

struct HistoryItem {
    QString prompt;
    QString filePath;
    QString date;
};

class HistoryService : public QObject {
    Q_OBJECT
public:
    explicit HistoryService(QObject *parent = nullptr);
    void load();
    void add(const QString &prompt, const QString &path);
    void remove(int index);
    QList<HistoryItem> getItems() const;
    QString getSavePath() const;

private:
    QList<HistoryItem> items;
    void saveJson();
};

#endif // HISTORYSERVICE_H