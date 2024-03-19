#ifndef STARTUPPOPUP_H
#define STARTUPPOPUP_H

#include "frame.h"

#include <QDialog>
#include <QLabel>

class QGroupBox;
class QVBoxLayout;

class StartupLabel : public QLabel {
    Q_OBJECT
public:
    explicit StartupLabel(const QString& text = "", QWidget* parent = 0,
        int index = -1);
    ~StartupLabel();
    QString m_text;
    int m_index;
signals:
    void wasClicked(int index);

protected:
    void mousePressEvent(QMouseEvent* event);
};

class StartupPopup : public QDialog
{
    Q_OBJECT

public:
    StartupPopup(QWidget* parent = nullptr);

    ~StartupPopup();
    QString getFilePath() { return m_filePath; }
    QString getFileName() { return m_fileName; }
    std::vector<QString> getRecentFiles() { return m_sceneNames; }
    static std::pair<QString, QString> openFile();


private:
    const int RECENT_SCENES_MAX_COUNT = 5;
    void openButtonPressed();
    void newButtonPressed();
    void recoverButtonPressed();
    bool m_validFile = false;
    QString m_filePath;
    QString m_fileName;

    QGroupBox* m_recentBox;
    QVBoxLayout* m_recentSceneLay;
    std::vector<StartupLabel*> m_recentNamesLabels;
    std::vector<QString> m_sceneNames;

    void loadRecentFiles();
    void refreshRecentScenes();
    bool checkValidFile(QString fileName);

    std::vector<Frame> m_tempFrames;
    bool saveXml(QString filePath, bool webcam);

public slots:
    void onRecentSceneClicked(int index);
};




#endif // STARTUPPOPUP_H
