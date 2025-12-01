#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "const/QtHeaders.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // 获取设置值
    QString getApiKey() const;
    QString getSubmitUrl() const;
    QString getQueryUrl() const;

signals:
    void settingsChanged();  // 设置发生变化时发射

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onResetClicked();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    // UI 组件
    QLineEdit *apiKeyEdit;
    QLineEdit *submitUrlEdit;
    QLineEdit *queryUrlEdit;
    QPushButton *saveBtn;
    QPushButton *cancelBtn;
    QPushButton *resetBtn;
    QLabel *statusLabel;
};

#endif // SETTINGSDIALOG_H

