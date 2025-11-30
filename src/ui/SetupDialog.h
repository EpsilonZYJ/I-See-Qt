#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "const/QtHeaders.h"

class SetupDialog : public QDialog {
    Q_OBJECT

public:
    SetupDialog(QWidget *parent = nullptr);

private slots:
    void selectFolder();

private:
    QLabel *infoLabel;
    QPushButton *selectButton;
};

#endif // SETUPDIALOG_H