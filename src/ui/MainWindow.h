#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "const/QtHeaders.h"
#include "viewmodel/MainViewModel.h"

class TaskHistoryWindow;

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

private:
    void setupUi(); // setupUi 声明

    MainViewModel *viewModel;
    TaskHistoryWindow *taskHistoryWindow;

    // UI 指针
    QListWidget *historyList;
    QLineEdit *apiKeyEdit;
    QTextEdit *promptEdit;
    QPushButton *generateBtn;
    QPushButton *taskHistoryBtn;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QVideoWidget *videoWidget;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
};
#endif