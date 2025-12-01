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
    void onModeChanged(int index);  // 模式切换
    void onSelectImage();  // 选择首帧图片
    void onSelectLastImage();  // 选择尾帧图片

private:
    void setupUi(); // setupUi 声明
    QString extractTaskIdFromFileName(const QString &fileName) const; // 从文件名提取 task_id
    QString imageToBase64(const QString &imagePath) const;  // 将图片转换为 Base64
    void updateImagePreview(QLabel *label, const QString &imagePath);  // 更新图片预览

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

    // 图生视频相关
    QComboBox *modeSelector;  // 模式选择：文生视频/图生视频
    QWidget *imageInputWidget;  // 图片输入区域容器
    QPushButton *selectImageBtn;  // 选择首帧图片
    QPushButton *selectLastImageBtn;  // 选择尾帧图片（可选）
    QLabel *imagePreviewLabel;  // 首帧图片预览
    QLabel *lastImagePreviewLabel;  // 尾帧图片预览
    QString firstImagePath;  // 首帧图片路径
    QString lastImagePath;  // 尾帧图片路径

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