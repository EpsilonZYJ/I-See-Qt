//
// Created by 周煜杰 on 2025/11/30.
//

#include "SetupDialog.h"
#include "const/AppConfig.h"
#include "const/QtHeaders.h"

// 构造函数实现 (解决 Undefined symbol: SetupDialog::SetupDialog)
SetupDialog::SetupDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("欢迎使用");
    resize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // 图标
    QLabel *iconLabel = new QLabel("📁");
    iconLabel->setAlignment(Qt::AlignCenter);
    QFont f = iconLabel->font();
    f.setPointSize(60);
    iconLabel->setFont(f);

    // 标题
    QLabel *title = new QLabel("首次运行设置");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 18px; font-weight: bold;");

    // 说明
    infoLabel = new QLabel("请选择视频的默认保存路径。\nApp 会记住此选择。");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("color: gray;");

    // 按钮
    selectButton = new QPushButton("选择文件夹...");
    selectButton->setMinimumHeight(45);
    selectButton->setStyleSheet("font-size: 14px;");

    layout->addStretch();
    layout->addWidget(iconLabel);
    layout->addWidget(title);
    layout->addWidget(infoLabel);
    layout->addWidget(selectButton);
    layout->addStretch();

    connect(selectButton, &QPushButton::clicked, this, &SetupDialog::selectFolder);
}

// 选择文件夹实现 (解决 Undefined symbol: SetupDialog::selectFolder)
void SetupDialog::selectFolder() {
    QString dir = QFileDialog::getExistingDirectory(this,
                  "选择视频保存路径",
                  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        // 保存到 QSettings
        QSettings settings(Config::ORG_NAME, Config::APP_NAME);
        settings.setValue(Config::KEY_SAVE_PATH, dir);

        // 关闭对话框，返回 Accepted
        accept();
    }
}