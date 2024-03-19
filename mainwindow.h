#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QGridLayout;
class StopMotionController;
class StopMotion;
class Viewer;
class Controls;
class Timeline;
class Director;
class TopBar;
class QDockWidget;
class Undo;
class AboutPopup;
class ShortcutPopup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool getSound() { return m_sound; }
    void setProject(QString filename, QString folderPath, std::vector<QString> recentFiles, bool existing = false);

private:
    Director* m_director;
    QGridLayout *m_mainLayout;
    StopMotionController* m_stopMotionController;
    StopMotion *m_stopMotion;
    Viewer* m_viewer;
    Controls* m_controls;
    Timeline* m_timeline;
    TopBar* m_topBar;
    QDockWidget* m_controllerDock;
    Undo* m_undo;
    AboutPopup* m_aboutPopup;
    ShortcutPopup* m_shortcutPopup;
    bool m_looping = false;
    bool m_startupPopupShown = false;

    void closeEvent(QCloseEvent* event) override;
    void moveEvent(QMoveEvent* event);

    void updateTitle();

    void toggleLooping();
    void registerCommands();
    void toggleSettings();

    bool m_sound = true;

private slots:
    void pathToolPressed(bool ignore);
    void moveToolPressed(bool ignore);
    void rectangleToolPressed(bool ignore);
    void lineToolPressed(bool ignore);
    void circleToolPressed(bool ignore);
    void textToolPressed(bool ignore);
    void onZoomPressed(bool ignore);
    void onPickZoomPressed(bool ignore);
    void onHighQualityToggled(bool ignore);
    void toggleLiveView(bool ignore);
    void onAbout(bool ignore);
    void toggleSound(bool ignore);
    void showShortcutPopup(bool ignore);
    void takeFocus();

};
#endif // MAINWINDOW_H
