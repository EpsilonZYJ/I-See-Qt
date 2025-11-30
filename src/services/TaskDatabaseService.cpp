#include "TaskDatabaseService.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

TaskDatabaseService::TaskDatabaseService(QObject *parent) : QObject(parent) {
}

TaskDatabaseService::~TaskDatabaseService() {
    if (db.isOpen()) {
        db.close();
    }
}

bool TaskDatabaseService::initialize() {
    // 获取应用数据目录
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir;
    if (!dir.exists(dataPath)) {
        dir.mkpath(dataPath);
    }

    QString dbPath = dataPath + "/tasks.db";
    qDebug() << "Database path:" << dbPath;

    // 创建数据库连接
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        emit databaseError("无法打开数据库: " + db.lastError().text());
        return false;
    }

    return createTables();
}

bool TaskDatabaseService::createTables() {
    QSqlQuery query(db);

    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            task_id TEXT PRIMARY KEY,
            prompt TEXT NOT NULL,
            api_key TEXT,
            width INTEGER,
            height INTEGER,
            resolution TEXT,
            aspect_ratio TEXT,
            duration INTEGER,
            camera_fixed INTEGER,
            seed INTEGER,
            status INTEGER,
            error_message TEXT,
            video_url TEXT,
            local_file_path TEXT,
            create_time TEXT,
            update_time TEXT,
            complete_time TEXT
        )
    )";

    if (!query.exec(createTableSQL)) {
        emit databaseError("创建表失败: " + query.lastError().text());
        return false;
    }

    // 创建索引以提高查询性能
    query.exec("CREATE INDEX IF NOT EXISTS idx_create_time ON tasks(create_time DESC)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_status ON tasks(status)");

    return true;
}

bool TaskDatabaseService::saveTask(const TaskItem &task) {
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO tasks (
            task_id, prompt, api_key, width, height, resolution, aspect_ratio,
            duration, camera_fixed, seed, status, error_message, video_url,
            local_file_path, create_time, update_time, complete_time
        ) VALUES (
            :task_id, :prompt, :api_key, :width, :height, :resolution, :aspect_ratio,
            :duration, :camera_fixed, :seed, :status, :error_message, :video_url,
            :local_file_path, :create_time, :update_time, :complete_time
        )
    )");

    query.bindValue(":task_id", task.taskId);
    query.bindValue(":prompt", task.prompt);
    query.bindValue(":api_key", task.apiKey);
    query.bindValue(":width", task.width);
    query.bindValue(":height", task.height);
    query.bindValue(":resolution", task.resolution);
    query.bindValue(":aspect_ratio", task.aspectRatio);
    query.bindValue(":duration", task.duration);
    query.bindValue(":camera_fixed", task.cameraFixed ? 1 : 0);
    query.bindValue(":seed", task.seed);
    query.bindValue(":status", static_cast<int>(task.status));
    query.bindValue(":error_message", task.errorMessage);
    query.bindValue(":video_url", task.videoUrl);
    query.bindValue(":local_file_path", task.localFilePath);
    query.bindValue(":create_time", task.createTime.toString(Qt::ISODate));
    query.bindValue(":update_time", task.updateTime.toString(Qt::ISODate));
    query.bindValue(":complete_time", task.completeTime.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "Save task error:" << query.lastError().text();
        emit databaseError("保存任务失败: " + query.lastError().text());
        return false;
    }

    return true;
}

bool TaskDatabaseService::updateTask(const TaskItem &task) {
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE tasks SET
            prompt = :prompt,
            status = :status,
            error_message = :error_message,
            video_url = :video_url,
            local_file_path = :local_file_path,
            update_time = :update_time,
            complete_time = :complete_time
        WHERE task_id = :task_id
    )");

    query.bindValue(":task_id", task.taskId);
    query.bindValue(":prompt", task.prompt);
    query.bindValue(":status", static_cast<int>(task.status));
    query.bindValue(":error_message", task.errorMessage);
    query.bindValue(":video_url", task.videoUrl);
    query.bindValue(":local_file_path", task.localFilePath);
    query.bindValue(":update_time", task.updateTime.toString(Qt::ISODate));
    query.bindValue(":complete_time", task.completeTime.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "Update task error:" << query.lastError().text();
        emit databaseError("更新任务失败: " + query.lastError().text());
        return false;
    }

    return true;
}

TaskItem TaskDatabaseService::getTask(const QString &taskId) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM tasks WHERE task_id = :task_id");
    query.bindValue(":task_id", taskId);

    if (query.exec() && query.next()) {
        return taskFromQuery(query);
    }

    return TaskItem();
}

QList<TaskItem> TaskDatabaseService::getAllTasks() {
    QList<TaskItem> tasks;
    QSqlQuery query(db);

    if (query.exec("SELECT * FROM tasks ORDER BY create_time DESC")) {
        while (query.next()) {
            tasks.append(taskFromQuery(query));
        }
    }

    return tasks;
}

QList<TaskItem> TaskDatabaseService::getPendingTasks() {
    QList<TaskItem> tasks;
    QSqlQuery query(db);

    // 状态 0 = Pending, 1 = Processing
    if (query.exec("SELECT * FROM tasks WHERE status IN (0, 1) ORDER BY create_time DESC")) {
        while (query.next()) {
            tasks.append(taskFromQuery(query));
        }
    }

    return tasks;
}

bool TaskDatabaseService::deleteTask(const QString &taskId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM tasks WHERE task_id = :task_id");
    query.bindValue(":task_id", taskId);

    if (!query.exec()) {
        emit databaseError("删除任务失败: " + query.lastError().text());
        return false;
    }

    return true;
}

TaskItem TaskDatabaseService::taskFromQuery(QSqlQuery &query) {
    TaskItem task;
    task.taskId = query.value("task_id").toString();
    task.prompt = query.value("prompt").toString();
    task.apiKey = query.value("api_key").toString();
    task.width = query.value("width").toInt();
    task.height = query.value("height").toInt();
    task.resolution = query.value("resolution").toString();
    task.aspectRatio = query.value("aspect_ratio").toString();
    task.duration = query.value("duration").toInt();
    task.cameraFixed = query.value("camera_fixed").toInt() == 1;
    task.seed = query.value("seed").toInt();
    task.status = static_cast<TaskStatus>(query.value("status").toInt());
    task.errorMessage = query.value("error_message").toString();
    task.videoUrl = query.value("video_url").toString();
    task.localFilePath = query.value("local_file_path").toString();
    task.createTime = QDateTime::fromString(query.value("create_time").toString(), Qt::ISODate);
    task.updateTime = QDateTime::fromString(query.value("update_time").toString(), Qt::ISODate);
    task.completeTime = QDateTime::fromString(query.value("complete_time").toString(), Qt::ISODate);

    return task;
}

