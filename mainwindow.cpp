#include "mainwindow.h"
#include "viewer.h"
#include "controls.h"
#include "timeline.h"
#include "commands.h"
#include "frame.h"
#include "director.h"
#include "startuppopup.h"
#include "aboutpopup.h"
#include "shortcutpopup.h"

#include <QGridLayout>
#include <QLabel>
#include <QFile>
#include <QApplication>
#include <QDockWidget>
#include <QFrame>

#include "stopmotion/stopmotion.h"
#include "stopmotion/stopmotioncontroller.h"
#include "topbar.h"
#include "undo.h"

//#include <QDesktopWidget>
#include <QAction>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    int h = this->screen()->availableVirtualGeometry().height();
    if (h < 768) {
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
    }
    m_director = Director::instance();
    m_director->setMainWindow(this);
    m_viewer = new Viewer(this);

    m_director->setViewer(m_viewer);
    m_stopMotion = StopMotion::instance();
    m_undo = Undo::instance();
    registerCommands();
    m_topBar = new TopBar(this);
    addToolBar(m_topBar);
    
    setMinimumWidth(1366);
    setMinimumHeight(700);
    m_stopMotionController = new StopMotionController(this);
    m_controllerDock = new QDockWidget(tr("Controller"), this);
    m_controllerDock->setWidget(m_stopMotionController);
    m_controllerDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_controllerDock->setTitleBarWidget(new QWidget(m_controllerDock));
    m_controllerDock->setFixedWidth(300);
    addDockWidget(Qt::RightDockWidgetArea, m_controllerDock);

    m_mainLayout = new QGridLayout(this);
    //m_mainLayout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    QFrame* mainFrame = new QFrame(this);
    mainFrame->setLayout(m_mainLayout);
    QDockWidget* dock = new QDockWidget(tr("Main"), this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    dock->setTitleBarWidget(new QWidget(dock));
    dock->setWidget(mainFrame);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    m_mainLayout->addWidget(m_viewer, 0, 0);

    m_controls = new Controls(this);
    m_mainLayout->addWidget(m_controls, 1, 0);

    m_timeline = new Timeline(this);
    m_mainLayout->addWidget(m_timeline, 2, 0);
    m_director->setFrameViewer(m_timeline->getFrameViewer());

    m_mainLayout->setRowStretch(0, 5000);

    showMaximized();

    m_aboutPopup = new AboutPopup(this);
    m_shortcutPopup = new ShortcutPopup(this);

    connect(m_director, &Director::updateTitle, this, &MainWindow::updateTitle);
    connect(m_shortcutPopup, &ShortcutPopup::takeFocus, this, &MainWindow::takeFocus);
}

MainWindow::~MainWindow()
{
    //delete m_mainLayout;
}

void MainWindow::moveEvent(QMoveEvent* event) {
    int h = this->screen()->geometry().height();
    double s = this->screen()->devicePixelRatio();
    if (h < 768) {
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
    }
    else {
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_director->getSceneDirty()) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Arlo",
            tr("Save before closing?\n"),
            QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
            QMessageBox::Save);
        if (resBtn == QMessageBox::Save) {
            m_director->saveProject();
            event->accept();
        }
        else if (resBtn == QMessageBox::Discard) {
            event->accept();
        }
        else if (resBtn == QMessageBox::Cancel) {
            event->ignore();
        }
        else {
            event->ignore();
        }
    }
    else {
        event->accept();
    }
}

void MainWindow::takeFocus() {
    setFocus();
    activateWindow();
}

void MainWindow::updateTitle() {
    QString path = m_stopMotion->getLevelName();
    QString name;
    if (path != "Untitled") {
        name = QFileInfo(path).baseName();
    }
    else {
        name = "Untitled";
    }
    if (m_director->getSceneDirty()) {
        setWindowTitle("Arlo - " + name + "*");
    }
    else {
        setWindowTitle("Arlo - " + name);
    }
}

void MainWindow::setProject(QString filename, QString folderPath, std::vector<QString> recentFiles, bool existing) {
    m_director->setProject(filename, folderPath, recentFiles, existing);
}

void MainWindow::toggleLooping() {
    m_looping = !m_looping;
    m_director->m_looping = !m_director->m_looping;
    Commands::instance()->getAction("Loop")->setChecked(m_director->m_looping);
}

void MainWindow::toggleSettings() {
    m_controllerDock->isVisible() ? m_controllerDock->hide() : m_controllerDock->show();
}

void MainWindow::toggleSound(bool ignore) {
    m_sound = !m_sound;
    if (m_sound) {
        Commands::instance()->getAction("Toggle Sound")->setIcon(QIcon(":/Resources/sound_on.svg"));
    }
    else {
        Commands::instance()->getAction("Toggle Sound")->setIcon(QIcon(":/Resources/sound_off.svg"));
    }
}

void MainWindow::showShortcutPopup(bool ignore) {
    m_shortcutPopup->show();
    m_shortcutPopup->clearFocus();
    this->activateWindow();
}

void MainWindow::moveToolPressed(bool ignore) {
    Commands::instance()->getAction("Move")->setChecked(true);
    Commands::instance()->getAction("Path")->setChecked(false);
    Commands::instance()->getAction("Line")->setChecked(false);
    Commands::instance()->getAction("Circle")->setChecked(false);
    Commands::instance()->getAction("Rectangle")->setChecked(false);
    Commands::instance()->getAction("Text")->setChecked(false);
    m_director->setActiveTool(Director::MOVE);
}
void MainWindow::pathToolPressed(bool ignore) {
    Commands::instance()->getAction("Move")->setChecked(false);
    Commands::instance()->getAction("Path")->setChecked(true);
    Commands::instance()->getAction("Line")->setChecked(false);
    Commands::instance()->getAction("Circle")->setChecked(false);
    Commands::instance()->getAction("Rectangle")->setChecked(false);
    Commands::instance()->getAction("Text")->setChecked(false);
    m_director->setActiveTool(Director::PATH);
}

void MainWindow::lineToolPressed(bool ignore) {
    Commands::instance()->getAction("Move")->setChecked(false);
    Commands::instance()->getAction("Path")->setChecked(false);
    Commands::instance()->getAction("Line")->setChecked(true);
    Commands::instance()->getAction("Circle")->setChecked(false);
    Commands::instance()->getAction("Rectangle")->setChecked(false);
    Commands::instance()->getAction("Text")->setChecked(false);
    m_director->setActiveTool(Director::LINE);
}

void MainWindow::circleToolPressed(bool ignore) {
    Commands::instance()->getAction("Move")->setChecked(false);
    Commands::instance()->getAction("Path")->setChecked(false);
    Commands::instance()->getAction("Line")->setChecked(false);
    Commands::instance()->getAction("Circle")->setChecked(true);
    Commands::instance()->getAction("Rectangle")->setChecked(false);
    Commands::instance()->getAction("Text")->setChecked(false);
    m_director->setActiveTool(Director::CIRCLE);
}
void MainWindow::rectangleToolPressed(bool ignore) {
    Commands::instance()->getAction("Move")->setChecked(false);
    Commands::instance()->getAction("Path")->setChecked(false);
    Commands::instance()->getAction("Line")->setChecked(false);
    Commands::instance()->getAction("Circle")->setChecked(false);
    Commands::instance()->getAction("Rectangle")->setChecked(true);
    Commands::instance()->getAction("Text")->setChecked(false);
    m_director->setActiveTool(Director::RECTANGLE);
}
void MainWindow::textToolPressed(bool ignore) {
    Commands::instance()->getAction("Move")->setChecked(false);
    Commands::instance()->getAction("Path")->setChecked(false);
    Commands::instance()->getAction("Line")->setChecked(false);
    Commands::instance()->getAction("Circle")->setChecked(false);
    Commands::instance()->getAction("Rectangle")->setChecked(false);
    Commands::instance()->getAction("Text")->setChecked(true);
    m_director->setActiveTool(Director::TEXT);
}

//-----------------------------------------------------------------------------

void MainWindow::onZoomPressed(bool ignore) {

    m_stopMotion->m_canon->zoomLiveView();

}

//-----------------------------------------------------------------------------

void MainWindow::onPickZoomPressed(bool ignore) {

    m_stopMotion->m_canon->toggleZoomPicking();

}

//-----------------------------------------------------------------------------

void MainWindow::onHighQualityToggled(bool ignore) {

    Director::instance()->toggleAlwaysUseLiveViewImages();

}

//-----------------------------------------------------------------------------

void MainWindow::onAbout(bool ignore) { m_aboutPopup->exec(); }

//-----------------------------------------------------------------------------

void MainWindow::toggleLiveView(bool ignore) {
    m_stopMotion->toggleLiveView();
}

//-----------------------------------------------------------------------------

void MainWindow::registerCommands() {
    Commands *cm = Commands::instance();

    //QAction* fitViewerAct = new QAction(tr("Fit View"), this);
    //QList<QKeySequence>fitViewerShortcuts;
    //QKeySequence fitViewer = Qt::Key_R;
    //fitViewerShortcuts.append(fitViewer);
    //fitViewerAct->setShortcuts(fitViewerShortcuts);
    //cm->addAction(fitViewerAct);
    //addAction(fitViewerAct);
    //connect(fitViewerAct, &QAction::triggered, m_viewer, &Viewer::fitViewer);

    QAction* playAct = new QAction(QIcon(":/Resources/play.svg"), tr("Play"), this);
    playAct->setStatusTip(tr("Loop Playback"));
    QList<QKeySequence>shortcuts;
    QKeySequence play = Qt::Key_0;
    QKeySequence play2 = Qt::Key_Space;
    QKeySequence play3 = Qt::Key_P;
    shortcuts.append(play);
    shortcuts.append(play2);
    shortcuts.append(play3);
    playAct->setShortcuts(shortcuts);
    //playAct->setShortcut(Qt::Key_0);
    cm->addAction(playAct);
    connect(playAct, &QAction::triggered, m_director, &Director::togglePlay);

    QAction* loopAct = new QAction(QIcon(":/Resources/loop.svg"), tr("Loop"), this);
    loopAct->setStatusTip(tr("Loop Playback"));
    QList<QKeySequence>loopShortcuts;
    QKeySequence loop1 = Qt::Key_L;
    QKeySequence loop2 = Qt::Key_8;
    loopShortcuts.append(loop1);
    loopShortcuts.append(loop2);
    loopAct->setShortcuts(loopShortcuts);
    loopAct->setCheckable(true);
    loopAct->setChecked(m_looping);
    cm->addAction(loopAct);
    connect(loopAct, &QAction::triggered, this, &MainWindow::toggleLooping);


    QAction* shortPlayAct = new QAction(QIcon(":/Resources/shortplay.svg"), tr("Short Play"), this);
    shortPlayAct->setStatusTip(tr("Play last 10 frames"));
    shortPlayAct->setAutoRepeat(false);
    QList<QKeySequence>sPlayshortcuts;
    QKeySequence sPlay = Qt::Key_Period;
    sPlayshortcuts.append(sPlay);
    shortPlayAct->setShortcuts(sPlayshortcuts);
    shortPlayAct->setCheckable(false);
    cm->addAction(shortPlayAct);
    connect(shortPlayAct, &QAction::triggered, m_director, &Director::shortPlay);

    QAction* playFiftyAct = new QAction(tr("PlayFifty"), this);
    playFiftyAct->setToolTip(tr("Play last 50 frames"));
    playFiftyAct->setAutoRepeat(false);
    QList<QKeySequence>playFiftyShortcuts;
    QKeySequence playFifty = Qt::Key_Comma;
    playFiftyShortcuts.append(playFifty);
    playFiftyAct->setShortcuts(playFiftyShortcuts);
    playFiftyAct->setCheckable(false);
    connect(playFiftyAct, &QAction::triggered, m_director, &Director::playFifty);
    cm->addAction(playFiftyAct);
    addAction(playFiftyAct);

    QAction* prevFrameAct = new QAction(tr("Previous Frame"), this);
    QList<QKeySequence>prevShortcuts;
    QKeySequence prevArrow = Qt::Key_Left;
    QKeySequence prevNum = Qt::Key_1;
    prevShortcuts.append(prevArrow);
    prevShortcuts.append(prevNum);
    prevFrameAct->setShortcuts(prevShortcuts);
    cm->addAction(prevFrameAct);
    connect(prevFrameAct, &QAction::triggered, m_director, &Director::previousFrame);
    addAction(prevFrameAct);


    QAction* nextFrameAct = new QAction(tr("Next Frame"), this);
    QList<QKeySequence>nextShortcuts;
    QKeySequence nextArrow = Qt::Key_Right;
    QKeySequence nextNum = Qt::Key_2;
    nextShortcuts.append(nextArrow);
    nextShortcuts.append(nextNum);
    nextFrameAct->setShortcuts(nextShortcuts);
    cm->addAction(nextFrameAct);
    connect(nextFrameAct, &QAction::triggered, m_director, &Director::nextFrame);
    addAction(nextFrameAct);

    QAction* firstFrameAct = new QAction(tr("First Frame"), this);
    QList<QKeySequence>firstShortcuts;
    QKeySequence firstArrow = Qt::Key_Up;
    QKeySequence homeButton = Qt::Key_Home;
    firstShortcuts.append(firstArrow);
    firstShortcuts.append(homeButton);
    firstFrameAct->setShortcuts(firstShortcuts);
    cm->addAction(firstFrameAct);
    connect(firstFrameAct, &QAction::triggered, m_director, &Director::firstFrame);
    addAction(firstFrameAct);

    QAction* lastFrameAct = new QAction(tr("Last Frame"), this);
    QList<QKeySequence>lastShortcuts;
    QKeySequence lastArrow = Qt::Key_Down;
    QKeySequence endButton = Qt::Key_End;
    lastShortcuts.append(lastArrow);
    lastShortcuts.append(endButton);
    lastFrameAct->setShortcuts(lastShortcuts);
    cm->addAction(lastFrameAct);
    connect(lastFrameAct, &QAction::triggered, m_director, &Director::lastFrame);
    addAction(lastFrameAct);

    QAction* jumpCameraAct = new QAction(tr("Jump to Camera"), this);
    QList<QKeySequence>jumpShortcuts;
    QKeySequence jumpNum = Qt::Key_3;
    jumpShortcuts.append(jumpNum);
    jumpCameraAct->setShortcuts(jumpShortcuts);
    cm->addAction(jumpCameraAct);
    connect(jumpCameraAct, &QAction::triggered, m_director, &Director::jumpToCameraFrame);
    addAction(jumpCameraAct);

    QAction* raiseOpacity = new QAction(tr("Raise Opacity"), this);
    QList<QKeySequence>raiseShortcuts;
    QKeySequence raise1 = Qt::Key_Plus;
    QKeySequence raise2 = Qt::Key_Equal;
    raiseShortcuts.append(raise1);
    raiseShortcuts.append(raise2);
    raiseOpacity->setShortcuts(raiseShortcuts);
    cm->addAction(raiseOpacity);
    connect(raiseOpacity, &QAction::triggered, m_director, &Director::raiseOpacity);
    addAction(raiseOpacity);

    QAction* lowerOpacity = new QAction(tr("Lower Opacity"), this);
    QList<QKeySequence>lowerShortcuts;
    QKeySequence lower1 = Qt::Key_Minus;
    lowerShortcuts.append(lower1);
    lowerOpacity->setShortcuts(lowerShortcuts);
    cm->addAction(lowerOpacity);
    connect(lowerOpacity, &QAction::triggered, m_director, &Director::lowerOpacity);
    addAction(lowerOpacity);

    QAction* saveAct = new QAction(tr("Save"), this);
    saveAct->setShortcut(QKeySequence("Ctrl+S"));
    cm->addAction(saveAct);
    addAction(saveAct);
    connect(saveAct, &QAction::triggered, m_director, &Director::saveProject);

    QAction* openAct = new QAction(tr("Open"), this);
    openAct->setShortcut(QKeySequence("Ctrl+O"));
    cm->addAction(openAct);
    addAction(openAct);
    connect(openAct, &QAction::triggered, m_director, &Director::openProject);

    QAction* newAct = new QAction(tr("New"), this);
    newAct->setShortcut(QKeySequence("Ctrl+N"));
    cm->addAction(newAct);
    addAction(newAct);
    connect(newAct, &QAction::triggered, m_director, &Director::newProject);

    QAction* settingsAct = new QAction(tr("Toggle Settings"), this);
    settingsAct->setShortcut(QKeySequence(Qt::Key_S));
    cm->addAction(settingsAct);
    addAction(settingsAct);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::toggleSettings);

    QAction* mp4Act = new QAction(tr("Export Video"), this);
    mp4Act->setShortcut(QKeySequence(Qt::Key_V));
    cm->addAction(mp4Act);
    addAction(mp4Act);
    connect(mp4Act, &QAction::triggered, m_director, &Director::makeMp4);

    QAction* deleteAct = new QAction(tr("Delete"), this);
    deleteAct->setShortcut(QKeySequence(Qt::Key_Delete));
    cm->addAction(deleteAct);
    addAction(deleteAct);
    connect(deleteAct, &QAction::triggered, m_director, &Director::deleteSomething);

    QAction* captureFrameAct = new QAction(tr("Capture"), this);
    QList<QKeySequence>captureShortcuts;
    QKeySequence cap1 = Qt::Key_Return;
    QKeySequence cap2 = Qt::Key_Enter;
    captureShortcuts.append(cap1);
    captureShortcuts.append(cap2);
    captureFrameAct->setShortcuts(captureShortcuts);
    //captureFrameAct->setShortcut(QKeySequence(Qt::Key_Delete));
    cm->addAction(captureFrameAct);
    addAction(captureFrameAct);
    connect(captureFrameAct, &QAction::triggered, m_stopMotion, &StopMotion::captureImage);

    QAction* cameraToEnd = new QAction(tr("Move Camera to the End"), this);
    cameraToEnd->setShortcut(QKeySequence("Ctrl+E"));
    cameraToEnd->setAutoRepeat(false);
    cm->addAction(cameraToEnd);
    addAction(cameraToEnd);
    connect(cameraToEnd, &QAction::triggered, m_director, &Director::cameraToEnd);

    QAction* insertCamera = new QAction(tr("Insert Camera"), this);
    insertCamera->setShortcut(QKeySequence("Ctrl+I"));
    insertCamera->setAutoRepeat(false);
    cm->addAction(insertCamera);
    addAction(insertCamera);
    connect(insertCamera, &QAction::triggered, m_director, &Director::insertCamera);

    QAction* moveTool = new QAction(QIcon(":/Resources/black_arrow.png"), tr("Move"), this);
    moveTool->setToolTip(tr("Move Tool"));
    moveTool->setAutoRepeat(false);
    QList<QKeySequence>moveShortcuts;
    //QKeySequence move1 = Qt::Key_1;
    //moveShortcuts.append(move1);
    //moveTool->setShortcuts(moveShortcuts);
    moveTool->setCheckable(true);
    moveTool->setChecked(true);
    cm->addAction(moveTool);
    connect(moveTool, &QAction::triggered, this, &MainWindow::moveToolPressed);

    QAction* rectangleTool = new QAction(QIcon(":/Resources/rectangle.svg"), tr("Rectangle"), this);
    rectangleTool->setAutoRepeat(false);
    rectangleTool->setToolTip(tr("Rectangle Tool"));
    //rectangleTool->setShortcut(Qt::Key_4);
    rectangleTool->setCheckable(true);
    cm->addAction(rectangleTool);
    connect(rectangleTool, &QAction::triggered, this, &MainWindow::rectangleToolPressed);

    QAction* lineTool = new QAction(QIcon(":/Resources/line.svg"), tr("Line"), this);
    lineTool->setToolTip(tr("Line Tool"));
    lineTool->setAutoRepeat(false);
    //lineTool->setShortcut(Qt::Key_3);
    lineTool->setCheckable(true);
    cm->addAction(lineTool);
    connect(lineTool, &QAction::triggered, this, &MainWindow::lineToolPressed);

    QAction* circleTool = new QAction(QIcon(":/Resources/circle.svg"), tr("Circle"), this);
    circleTool->setToolTip(tr("Circle Tool"));
    circleTool->setAutoRepeat(false);
    //circleTool->setShortcut(Qt::Key_5);
    circleTool->setCheckable(true);
    cm->addAction(circleTool);
    connect(circleTool, &QAction::triggered, this, &MainWindow::circleToolPressed);

    QAction* textTool = new QAction(QIcon(":/Resources/text_tool.png"), tr("Text"), this);
    textTool->setToolTip(tr("Text Tool"));
    textTool->setAutoRepeat(false);
    //textTool->setShortcut(Qt::Key_6);
    textTool->setCheckable(true);
    cm->addAction(textTool);
    connect(textTool, &QAction::triggered, this, &MainWindow::textToolPressed);

    QAction* pathTool = new QAction(QIcon(":/Resources/white_arrow.png"), tr("Path"), this);
    pathTool->setAutoRepeat(false);
    pathTool->setToolTip(tr("Path Tool"));
    //pathTool->setShortcut(Qt::Key_2);
    pathTool->setCheckable(true);
    cm->addAction(pathTool);
    connect(pathTool, &QAction::triggered, this, &MainWindow::pathToolPressed);

    QAction* undoAct = new QAction(QIcon(":/Resources/undo.png"), tr("Undo"), this);
    undoAct->setShortcut(QKeySequence("Ctrl+Z"));
    undoAct->setAutoRepeat(false);
    cm->addAction(undoAct);
    addAction(undoAct);
    connect(undoAct, &QAction::triggered, m_undo, &Undo::undoIt);

    QAction* copyAct = new QAction(tr("Copy"), this);
    copyAct->setShortcut(QKeySequence("Ctrl+C"));
    copyAct->setAutoRepeat(false);
    cm->addAction(copyAct);
    addAction(copyAct);
    connect(copyAct, &QAction::triggered, m_director, &Director::copy);

    QAction* pasteAct = new QAction(tr("Paste"), this);
    pasteAct->setShortcut(QKeySequence("Ctrl+V"));
    pasteAct->setAutoRepeat(false);
    cm->addAction(pasteAct);
    addAction(pasteAct);
    connect(pasteAct, &QAction::triggered, m_director, &Director::paste);

    QAction* cutAct = new QAction(tr("Cut"), this);
    cutAct->setShortcut(QKeySequence("Ctrl+X"));
    cutAct->setAutoRepeat(false);
    cm->addAction(cutAct);
    addAction(cutAct);
    connect(cutAct, &QAction::triggered, m_director, &Director::cut);

    QAction* redoAct = new QAction(QIcon(":/Resources/redo.png"), tr("Redo"), this);
    redoAct->setShortcut(QKeySequence("Ctrl+Y"));
    redoAct->setAutoRepeat(false);
    cm->addAction(redoAct);
    addAction(redoAct);
    connect(redoAct, &QAction::triggered, m_undo, &Undo::redo);

    QAction* duplicateAct = new QAction(tr("Duplicate Frame"), this);
    duplicateAct->setShortcut(QKeySequence("D"));
    duplicateAct->setAutoRepeat(false);
    cm->addAction(duplicateAct);
    addAction(duplicateAct);
    connect(duplicateAct, &QAction::triggered, m_director, &Director::duplicate);

    QAction* highQualityImages = new QAction(QIcon(":/Resources/highquality.svg"), tr("High Quality"), this);
    highQualityImages->setToolTip(tr("Show Final Images for Preview"));
    highQualityImages->setAutoRepeat(false);
    highQualityImages->setShortcut(Qt::Key_6);
    highQualityImages->setCheckable(true);
    highQualityImages->setChecked(false);
    cm->addAction(highQualityImages);
    connect(highQualityImages, &QAction::triggered, this, &MainWindow::onHighQualityToggled);
     
    QAction* focusCheck = new QAction(QIcon(":/Resources/focus.svg"), tr("Focus"), this);
    focusCheck->setToolTip(tr("Focus Check"));
    focusCheck->setAutoRepeat(false);
    focusCheck->setShortcut(QKeySequence("*"));
    focusCheck->setCheckable(true);
    cm->addAction(focusCheck);
    connect(focusCheck, &QAction::triggered, this, &MainWindow::onZoomPressed);

    QAction* pickFocusCheck = new QAction(QIcon(":/Resources/pick.svg"), tr("Pick Focus"), this);
    pickFocusCheck->setToolTip(tr("Pick Focus Check Area"));
    pickFocusCheck->setAutoRepeat(false);
    pickFocusCheck->setShortcut(QKeySequence("/"));
    pickFocusCheck->setCheckable(true);
    cm->addAction(pickFocusCheck);
    connect(pickFocusCheck, &QAction::triggered, this, &MainWindow::onPickZoomPressed);

    QAction* toggleLiveView = new QAction(tr("Toggle Live View"), this);
    toggleLiveView->setToolTip(tr("Toggle Live View"));
    toggleLiveView->setAutoRepeat(false);
    toggleLiveView->setShortcut(QKeySequence("5"));
    toggleLiveView->setCheckable(false);
    cm->addAction(toggleLiveView);
    addAction(toggleLiveView);
    connect(toggleLiveView, &QAction::triggered, this, &MainWindow::toggleLiveView);

    QAction* flipHoriz = new QAction(QIcon(":/Resources/flip.svg"), tr("Flip Horizontally"), this);
    flipHoriz->setToolTip(tr("Flip Horizontally"));
    flipHoriz->setAutoRepeat(false);
    flipHoriz->setCheckable(true);
    cm->addAction(flipHoriz);
    connect(flipHoriz, &QAction::triggered, m_director, &Director::toggleFlip);

    QAction* ruleOfThirds = new QAction(QIcon(":/Resources/ruleofthirds.svg"), tr("Rule of Thirds"), this);
    ruleOfThirds->setToolTip(tr("Show the Rule of Thirds"));
    ruleOfThirds->setAutoRepeat(false);
    ruleOfThirds->setCheckable(true);
    cm->addAction(ruleOfThirds);
    connect(ruleOfThirds, &QAction::triggered, m_director, &Director::toggleRuleOfThirds);

    QAction* showOverlays = new QAction(QIcon(":/Resources/overlays.svg"), tr("Overlays"), this);
    showOverlays->setToolTip(tr("Show Overlays"));
    showOverlays->setAutoRepeat(false);
    showOverlays->setCheckable(true);
    cm->addAction(showOverlays);
    connect(showOverlays, &QAction::triggered, m_director, &Director::toggleOverlays);

    QAction* imgSeq = new QAction(tr("Export Image Sequence"), this);
    imgSeq->setToolTip(tr("Export Image Sequence"));
    imgSeq->setAutoRepeat(false);
    imgSeq->setCheckable(false);
    cm->addAction(imgSeq);
    addAction(imgSeq);
    connect(imgSeq, &QAction::triggered, m_director, &Director::exportSequence);

    QAction* muteAct = new QAction(QIcon(":/Resources/sound_on.svg"), tr("Toggle Sound"), this);
    muteAct->setToolTip(tr("Toggle Sound"));
    muteAct->setShortcut(Qt::Key_7);
    muteAct->setAutoRepeat(false);
    muteAct->setCheckable(false);
    cm->addAction(muteAct);
    addAction(muteAct);
    connect(muteAct, &QAction::triggered, this, &MainWindow::toggleSound);

    QAction* resetViewer = new QAction(QIcon(":/Resources/resetviewer.png"), tr("Reset Viewer Position"), this);
    resetViewer->setToolTip(tr("Reset Viewer Position"));
    resetViewer->setShortcut(Qt::Key_R);
    resetViewer->setAutoRepeat(false);
    resetViewer->setCheckable(false);
    cm->addAction(resetViewer);
    addAction(resetViewer);
    connect(resetViewer, &QAction::triggered, m_viewer, &Viewer::fitViewer);

    QAction* conform = new QAction(tr("Conform"), this);
    conform->setToolTip(tr("Conform the current take"));
    conform->setAutoRepeat(false);
    conform->setShortcut(QKeySequence("`"));
    conform->setCheckable(false);
    cm->addAction(conform);
    addAction(conform);
    connect(conform, &QAction::triggered, m_director, &Director::conform);

    QAction* shortcutsAct = new QAction(tr("   ?   "), this);
    shortcutsAct->setToolTip(tr("Show Shortcut Window"));
    shortcutsAct->setAutoRepeat(false);
    shortcutsAct->setCheckable(false);
    shortcutsAct->setShortcut(QKeySequence("?"));
    cm->addAction(shortcutsAct);
    addAction(shortcutsAct);
    connect(shortcutsAct, &QAction::triggered, this, &MainWindow::showShortcutPopup);

    QAction* about = new QAction(tr("About"), this);
    about->setToolTip(tr("About Arlo"));
    about->setAutoRepeat(false);
    about->setCheckable(false);
    cm->addAction(about);
    addAction(about);
    connect(about, &QAction::triggered, this, &MainWindow::onAbout);
}