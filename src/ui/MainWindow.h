#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "const/QtHeaders.h"
#include "viewmodel/MainViewModel.h"

class TaskHistoryWindow;
class SettingsDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow(); // 析构函数声明

private slots:
    void onGenerateClicked();
    void updateHistoryList();
    void onVideoReady(const QString &path);
    void onShowTaskHistory();
    void onShowSettings();
    void onSettingsChanged();

private:
    void setupUi(); // setupUi 声明
    QString extractTaskIdFromFileName(const QString &fileName) const; // 从文件名提取 task_id

    MainViewModel *viewModel;
    TaskHistoryWindow *taskHistoryWindow;
    SettingsDialog *settingsDialog;

    // UI 指针
    QListWidget *historyList;
    QLineEdit *apiKeyEdit;  // 保留用于显示，但不可编辑
    QTextEdit *promptEdit;
    QPushButton *generateBtn;
    QPushButton *taskHistoryBtn;
    QPushButton *settingsBtn;  // 新增设置按钮
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QVideoWidget *videoWidget;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;

    // 参数配置相关
    QWidget *parametersWidget;
    QVBoxLayout *parametersLayout;
    QPushButton *addParameterBtn;

    struct ParameterRow {
        QLineEdit *nameEdit;
        QLineEdit *valueEdit;
        QPushButton *removeBtn;
        QHBoxLayout *layout;
    };
    QList<ParameterRow> parameterRows;

    void addParameterRow(const QString &name = "", const QString &value = "");
    void removeParameterRow(int index);
    QMap<QString, QString> getParameters() const;
};
#endif