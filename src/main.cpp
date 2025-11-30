#include "ui/MainWindow.h"
#include "ui/SetupDialog.h"
#include "const/AppConfig.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(Config::ORG_NAME);
    QCoreApplication::setApplicationName(Config::APP_NAME);

    QSettings settings;
    if (!settings.contains(Config::KEY_SAVE_PATH)) {
        SetupDialog setup;
        if (setup.exec() != QDialog::Accepted) {
            return 0;
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}