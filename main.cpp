#include "mainwindow.h"
#include "startuppopup.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QMessageBox>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "StopMo_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

#ifdef _WIN32
    QFont font("Segoe UI", -1);
#else
    QFont font("Helvetica", -1);
#endif
    font.setPixelSize(12);
    font.setWeight(QFont::Normal);
    a.setFont(font);

    QApplication::setStyle("windows");


    QString stylePath = QDir::currentPath();
    stylePath = ":/Resources/Darker.qss";
    QFile file(stylePath);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    file.close();
    a.setStyleSheet(styleSheet);
    



    StartupPopup startupPopup;
    startupPopup.show();
    int result = startupPopup.exec();
    bool existing = false;
    if ( result > 0) {
        QString fileName = startupPopup.getFileName();
        QString filePath = startupPopup.getFilePath();
        std::vector <QString> recentFiles = startupPopup.getRecentFiles();
        if (result == 2) {
            existing = true;
        }
        MainWindow w;
        w.setWindowTitle("Arlo");
        w.show();
        w.setProject(fileName, filePath, recentFiles, existing);
        return a.exec();
    }
    else return 0;

}
