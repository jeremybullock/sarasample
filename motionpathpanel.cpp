#include "motionpathpanel.h"
#include "motionpath.h"
#include "graphwidget.h"
#include "director.h"
#include "commands.h"

#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QToolBar>
#include <QPixmap>
#include <QSlider>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QLineEdit>
#include <QSizePolicy>
#include <QTimer>
#include <QColorDialog>
#include <QToolButton>

namespace {
double distanceSquared(QPoint p1, QPoint p2) {
  int newX = p1.x() - p2.x();
  int newY = p1.y() - p2.y();
  return (newX * newX) + (newY * newY);
}
};

//*****************************************************************************
//    MotionPathPanel  implementation
//*****************************************************************************

MotionPathPanel::MotionPathPanel(QWidget* parent)
    : QWidget(parent), m_playbackExecutor() {
  
  m_director = Director::instance();
  
  m_outsideLayout = new QGridLayout();
  m_outsideLayout->setContentsMargins(10, 10, 10, 10);
  m_outsideLayout->setSpacing(15);

  m_graphArea = new GraphWidget();
  m_graphArea->setMaxXValue(1000);
  m_graphArea->setMaxYValue(1000);
  m_graphArea->setPoints(m_currentPath.getIterpolationPoints());

  // make the toolbar
  m_toolbar = new QToolBar(this);

  QToolButton* moveButton = new QToolButton(this);
  moveButton->setDefaultAction(Commands::instance()->getAction("Move"));
  moveButton->setIconSize(QSize(25, 25));
  m_toolbar->addWidget(moveButton);

  QToolButton* pathButton = new QToolButton(this);
  pathButton->setDefaultAction(Commands::instance()->getAction("Path"));
  pathButton->setIconSize(QSize(25, 25));
  m_toolbar->addWidget(pathButton);

  QToolButton* lineButton = new QToolButton(this);
  lineButton->setDefaultAction(Commands::instance()->getAction("Line"));
  lineButton->setIconSize(QSize(25, 25));
  m_toolbar->addWidget(lineButton);

  QToolButton* rectangleButton = new QToolButton(this);
  rectangleButton->setDefaultAction(Commands::instance()->getAction("Rectangle"));
  rectangleButton->setIconSize(QSize(25, 25));
  m_toolbar->addWidget(rectangleButton);

  QToolButton* circleButton = new QToolButton(this);
  circleButton->setDefaultAction(Commands::instance()->getAction("Circle"));
  circleButton->setIconSize(QSize(25, 25));
  m_toolbar->addWidget(circleButton);

  //QToolButton* textButton = new QToolButton(this);
  //textButton->setDefaultAction(Commands::instance()->getAction("Text"));
  //textButton->setIconSize(QSize(25, 25));
  //m_toolbar->addWidget(textButton);

  m_outsideLayout->addWidget(m_toolbar, 5, 0, 1, 4);

  //QLineEdit* stepsEdit = new QLineEdit(this);
  //stepsEdit->setText(QString::number(m_currentPath.getStepCount()));
  ////stepsEdit->setRange(0, 300);
  //stepsEdit->setFixedWidth(40);

  m_stepsSlider = new QSlider(this);
  m_stepsSlider->setOrientation(Qt::Horizontal);
  m_stepsSlider->setRange(0, 150);
  m_stepsSlider->setValue(0);

  m_widthSlider = new QSlider(this);
  m_widthSlider->setOrientation(Qt::Horizontal);
  m_widthSlider->setRange(1, 6);
  m_widthSlider->setValue(m_currentPath.getWidth());
  //widthSlider->setFixedWidth(30);

  m_colorButton = new QPushButton(this);
  QPixmap pixmap(30, 20);
  pixmap.fill(Qt::red);
  QIcon blackIcon(pixmap);
  m_colorButton->setIcon(blackIcon);
  m_colorButton->setIconSize(pixmap.rect().size());

  connect(m_stepsSlider, &QSlider::valueChanged, [=]() {
      int steps = m_stepsSlider->value();
      m_currentPath.setStepCount(steps);
      m_director->setMotionPathStepCount(steps);
      m_stepsLabel->setText(tr("Steps: ") + QString::number(steps) + " ");
      if (steps > 0) {
          m_graphFrame->show();
          m_playToolbar->show();
      }
      else {
          m_graphFrame->hide();
          m_playToolbar->hide();
      }
      m_director->updatePaths();
      });
  connect(m_widthSlider, &QSlider::valueChanged, [=]() {
      int width = m_widthSlider->value();
      m_currentPath.setWidth(width);
      m_director->setMotionPathWidth(width);
      m_widthLabel->setText(tr("Width: ") + QString::number(width) + " ");
      m_director->updatePaths();
      });
  connect(m_colorButton, &QAbstractButton::clicked, this, &MotionPathPanel::setColor);

  m_colorLabel = new QLabel(tr("Color: "), this);
  m_widthLabel = new QLabel(tr("Width: 1 "), this);
  m_stepsLabel = new QLabel(tr("Steps: 0 "), this);

  m_outsideLayout->addWidget(m_stepsLabel, 3, 0, Qt::AlignRight);
  m_outsideLayout->addWidget(m_stepsSlider, 3, 1, 1, 3);
  m_outsideLayout->addWidget(m_widthLabel, 4, 0, Qt::AlignRight);
  m_outsideLayout->addWidget(m_widthSlider, 4, 1);
  m_outsideLayout->addWidget(m_colorLabel, 4, 2, Qt::AlignRight);
  m_outsideLayout->addWidget(m_colorButton, 4, 3);
  m_outsideLayout->setColumnStretch(1, 100);
   
  QHBoxLayout* graphLayout = new QHBoxLayout();
  graphLayout->setContentsMargins(0, 0, 0, 0);
  graphLayout->setSpacing(0);
  graphLayout->addWidget(m_graphArea);

  m_graphFrame = new QFrame(this);
  m_graphFrame->setLayout(graphLayout);
  m_graphFrame->setObjectName("GraphAreaFrame");


  m_outsideLayout->addWidget(m_graphFrame, 1, 0, 1, 4);
  m_graphFrame->hide();

  m_playToolbar = new QToolBar(this);
  m_playToolbar->setFixedHeight(18);
  m_playToolbar->setIconSize(QSize(16, 16));

  QAction* playAction = new QAction(this);
  playAction->setIcon(QIcon(":/Resources/play.svg"));
  m_playToolbar->addAction(playAction);
  connect(playAction, &QAction::triggered, [=]() {
      m_playReversed = false;
      if (!m_setLiveView) {
          m_showLiveViewTemp = Director::instance()->getShowLiveView();
          Director::instance()->setShowLiveView(false);
          m_setLiveView = true;
      }
    m_looping = false;
    int fps   = 24;
    fps = Director::instance()->getFps();;
    m_playbackExecutor.resetFps(fps);
    m_currentPath.setCurrentStep(0);
    m_director->setMotionPathCurrentStep(0);
    Director::instance()->setPathIsPlaying(true);
    if (!m_playbackExecutor.isRunning()) m_playbackExecutor.start();
    Director::instance()->nextPathFrame();
  });

  QAction* playReverseAction = new QAction(this);
  playReverseAction->setIcon(QIcon(":/Resources/playreverse.svg"));
  m_playToolbar->addAction(playReverseAction);

  connect(playReverseAction, &QAction::triggered, [=]() {
      m_playReversed = true;
      if (!m_setLiveView) {
          m_showLiveViewTemp = Director::instance()->getShowLiveView();
          Director::instance()->setShowLiveView(false);
          m_setLiveView = true;
      }
      m_looping = false;
      int fps = 24;
      fps = Director::instance()->getFps();;
      m_playbackExecutor.resetFps(fps);

      m_currentPath.setCurrentStep(m_currentPath.getStepCount() - 1);
      m_director->setMotionPathCurrentStep(m_currentPath.getStepCount() - 1);
      Director::instance()->setPathIsPlaying(true);
      if (!m_playbackExecutor.isRunning()) m_playbackExecutor.start();
      Director::instance()->nextPathFrame();
  });

  QAction* loopAction = new QAction(this);
  loopAction->setIcon(QIcon(":/Resources/loop.svg"));
  m_playToolbar->addAction(loopAction);
  connect(loopAction, &QAction::triggered, [=]() {
    if (!m_setLiveView) {
        m_showLiveViewTemp = Director::instance()->getShowLiveView();
        Director::instance()->setShowLiveView(false);
        m_setLiveView = true;
    }
    m_looping = true;
    int fps   = 24;
    fps       = Director::instance()->getFps();
    m_playbackExecutor.resetFps(fps);
    m_currentPath.setCurrentStep(0);
    m_director->setMotionPathCurrentStep(0);
    Director::instance()->setPathIsPlaying(true);
    Director::instance()->nextPathFrame();
    if (!m_playbackExecutor.isRunning()) m_playbackExecutor.start();
  });

  QAction* stopActionAction = new QAction(this);
  stopActionAction->setIcon(QIcon(":/Resources/stop.svg"));
  m_playToolbar->addAction(stopActionAction);
  connect(stopActionAction, &QAction::triggered, [=]() {
      if (m_setLiveView) {
          Director::instance()->setShowLiveView(m_showLiveViewTemp);
          m_setLiveView = false;
      }
      if (m_playbackExecutor.isRunning()) m_playbackExecutor.abort();
      m_looping = false;
      Director::instance()->setPathIsPlaying(false);
      m_currentPath.setCurrentStep(0);
      m_director->setMotionPathCurrentStep(0);
      Director::instance()->nextPathFrame();
  });

  m_outsideLayout->addWidget(m_playToolbar, 2, 0, 1, 4);
  m_playToolbar->hide();

  //m_outsideLayout->addLayout(m_controlsLayout);
  setLayout(m_outsideLayout);

  connect(m_graphArea, &GraphWidget::controlPointChanged, [=](bool dragging) {
      m_currentPath.setInterpolationPoints(m_graphArea->getPoints());
      m_director->setMotionPathInterpolationPoints(m_graphArea->getPoints(), dragging);
      Director::instance()->updatePaths();
      if (!dragging) m_graphArea->clearFocus();
  });

  bool ret = connect(&m_playbackExecutor, SIGNAL(nextFrame(int, QElapsedTimer*,
      qint64)), this,
                     SLOT(onNextFrame(int, QElapsedTimer*,
                         qint64)), Qt::BlockingQueuedConnection);

  ret = ret && connect(&m_playbackExecutor, SIGNAL(playbackAborted()), this,
                       SLOT(stopPlayback()));

  connect(Director::instance(), &Director::newPathSelected,
      this, &MotionPathPanel::onPathSelectedSignal);
  assert(ret);

  // hide everything to start
  m_graphFrame->hide();
  m_playToolbar->hide();
  m_colorLabel->hide();
  m_colorButton->hide();
  m_widthLabel->hide();
  m_widthSlider->hide();
  m_stepsLabel->hide();
  m_stepsSlider->hide();
}

//-----------------------------------------------------------------------------

MotionPathPanel::~MotionPathPanel() {}

//-----------------------------------------------------------------------------

void MotionPathPanel::showEvent(QShowEvent*) { } 

//-----------------------------------------------------------------------------

//void MotionPathPanel::onNewPathSignal() {
//    m_currentPath = Director::instance()->getMotionPaths().at(0);
//}

//------------------------------------------------------------------------------


void MotionPathPanel::onPathSelectedSignal(int path) {
    if (path < 0) {
        m_currentPath = m_placeHolderPath;
        m_graphFrame->hide();
        m_playToolbar->hide();
        m_colorLabel->hide();
        m_colorButton->hide();
        m_widthLabel->hide();
        m_widthSlider->hide();
        m_stepsLabel->hide();
        m_stepsSlider->hide();
        stopPlayback();
        return;
    }

    stopPlayback();
    m_currentPath = Director::instance()->getMotionPaths().at(path);
    QPixmap pixmap(30, 20);
    pixmap.fill(m_currentPath.getColor());
    QIcon redIcon(pixmap);
    m_colorButton->setIcon(redIcon);
    m_currentColor = m_currentPath.getColor();

    if (m_currentPath.getStepCount() > 0) {
        m_graphFrame->show();
        m_playToolbar->show();
    }
    else {
        m_graphFrame->hide();
        m_playToolbar->hide();
    }
    m_graphArea->setPoints(m_currentPath.getIterpolationPoints());
    m_colorLabel->show();
    m_colorButton->show();
    m_widthLabel->show();
    m_widthSlider->show();
    m_stepsLabel->show();
    m_stepsSlider->show();
    m_stepsLabel->setText(tr("Steps: ") + QString::number(m_currentPath.getStepCount()) + " ");
    m_widthLabel->setText(tr("Width: ") + QString::number(m_currentPath.getWidth()) + " ");
    m_stepsSlider->blockSignals(true);
    m_stepsSlider->setValue(m_currentPath.getStepCount());
    m_stepsSlider->blockSignals(false);
    
    m_widthSlider->blockSignals(true);
    m_widthSlider->setValue(m_currentPath.getWidth());
    m_widthSlider->blockSignals(false);

    m_graphArea->clearFocus();
}

//-----------------------------------------------------------------------------

void MotionPathPanel::setColor()
{
    const QColor color = QColorDialog::getColor(m_currentColor, this, "Select Color");
    if (color.isValid()) {
        QPixmap pixmap(30, 20);
        pixmap.fill(color);
        QIcon redIcon(pixmap);
        m_colorButton->setIcon(redIcon);
        m_currentColor = color;
        m_currentPath.setColor(color);
        m_director->setMotionPathColor(color);
    }
}

//-----------------------------------------------------------------------------

void MotionPathPanel::onNextFrame(int fps, QElapsedTimer* timer,
    qint64 targetInstant) {

  int steps       = m_currentPath.getStepCount();
  int currentStep = m_currentPath.getCurrentStep();
  if (m_looping) {
      m_currentPath.setCurrentStep(currentStep >= steps - 1 ? 0
          : currentStep + 1);
      m_director->setMotionPathCurrentStep(currentStep >= steps - 1 ? 0
          : currentStep + 1);
  }
  else {
      if (m_playReversed) {
          if (currentStep == 0) {
              m_currentPath.setCurrentStep(0);
              m_director->setMotionPathCurrentStep(0);
              Director::instance()->setPathIsPlaying(false);
              m_looping = false;
              m_playbackExecutor.abort();
          }
          else {
              m_currentPath.setCurrentStep(currentStep -1);
              m_director->setMotionPathCurrentStep(currentStep - 1);
          }
      }
      else {
          if (currentStep >= steps - 1) {
              m_currentPath.setCurrentStep(0);
              m_director->setMotionPathCurrentStep(0);
              Director::instance()->setPathIsPlaying(false);
              m_looping = false;
              m_playbackExecutor.abort();
          }
          else {
              m_currentPath.setCurrentStep(currentStep + 1);
              m_director->setMotionPathCurrentStep(currentStep + 1);
          }
      }
  }
  Director::instance()->nextPathFrame();
}
//-----------------------------------------------------------------------------

void MotionPathPanel::stopPlayback() {
    Director::instance()->setPathIsPlaying(false);
    Director::instance()->nextPathFrame();
    //m_playbackExecutor.abort();
    m_looping = false;
    if (m_setLiveView) {
        Director::instance()->setShowLiveView(m_showLiveViewTemp);
        m_setLiveView = false;
    }

    m_currentPath.setCurrentStep(0);
    m_director->setMotionPathCurrentStep(0);
}

//=============================================================================

ClickablePathLabel::ClickablePathLabel(const QString& text, QWidget* parent,
                                       Qt::WindowFlags f)
    : QLabel(text, parent, f) {
  setMaximumHeight(18);
  setObjectName("MotionPathLabel");
}

//-----------------------------------------------------------------------------

ClickablePathLabel::~ClickablePathLabel() {}

//-----------------------------------------------------------------------------

void ClickablePathLabel::mouseReleaseEvent(QMouseEvent* event) {
  emit onMouseRelease(event);
}

void ClickablePathLabel::mouseDoubleClickEvent(QMouseEvent* event) {
  emit doubleClicked();
}

//-----------------------------------------------------------------------------

//void ClickablePathLabel::enterEvent(QEvent*) {}
//
////-----------------------------------------------------------------------------
//
//void ClickablePathLabel::leaveEvent(QEvent*) {}

//-----------------------------------------------------------------------------

void ClickablePathLabel::setSelected() {}

//-----------------------------------------------------------------------------

void ClickablePathLabel::clearSelected() {}

//=============================================================================

MotionPathPlaybackExecutor::MotionPathPlaybackExecutor()
    : m_fps(25), m_abort(false) {}

//-----------------------------------------------------------------------------

void MotionPathPlaybackExecutor::resetFps(int fps) { m_fps = fps; }

//-----------------------------------------------------------------------------

void MotionPathPlaybackExecutor::run() {
    m_timer.start();

    qint64 timeResolution =
        250 * 1000000;  // Use a sufficient sampling resolution (currently 1/4
                        // sec). Fps calculation is made once per sample.

    int fps = m_fps, currSample = 0;
    qint64 playedFramesCount = 0;
    qint64 nextSampleInstant = timeResolution;

    qint64 lastFrameCounts[4] = { 0, 0, 0,
                                 0 };  // Keep the last 4 'played frames' counts.
    qint64 lastSampleInstants[4] = { 0, 0, 0,
                                    0 };  // Same for the last sampling instants

    qint64 targetFrameTime =
        1000000000 / (qint64)abs(m_fps);  // User-required time between frames

    qint64 emissionInstant = 0;  // starting instant in which rendering is invoked
    qint64 avgSwapTime = 0;  // average time for swapping buffers
    qint64 shortTermDelayAdjuster =
        0;  // accumurate recent errors and adjust in short term

    while (!m_abort) {
        emissionInstant = m_timer.nsecsElapsed();

        if (emissionInstant > nextSampleInstant) {
            // Fps calculation
            qint64 framesCount = playedFramesCount - lastFrameCounts[currSample];
            qint64 elapsedTime = emissionInstant - lastSampleInstants[currSample];
            fps = (int)(((long double)(1000000000 * framesCount) /
                (long double)elapsedTime) + 0.5);

            targetFrameTime =
                1000000000 / (qint64)abs(m_fps);  // m_fps could have changed...

            // estimate time for swapping buffers
            qint64 avgSwapTimeD = (elapsedTime / framesCount) - targetFrameTime;
            if (avgSwapTime - avgSwapTimeD >
                20000000)  // Reset beyond, say, 20 msecs tolerance.
                avgSwapTime = avgSwapTimeD;
            else
                avgSwapTime += avgSwapTimeD;
            avgSwapTime = std::min(targetFrameTime, std::max(avgSwapTime, (qint64)0));

            // prepare for the next sampling
            lastFrameCounts[currSample] = playedFramesCount;
            lastSampleInstants[currSample] = emissionInstant;
            currSample = (currSample + 1) % 4;
            nextSampleInstant = emissionInstant + timeResolution;
        }

        // draw the next frame
        if (playedFramesCount) {
            qint64 delayAdjust = shortTermDelayAdjuster / 4;
            qint64 targetInstant =
                emissionInstant + targetFrameTime - avgSwapTime - delayAdjust;
            targetInstant = std::max(targetInstant, emissionInstant);
            shortTermDelayAdjuster -= delayAdjust;

            // Show the next frame, telling currently measured fps
            // For the Flipbook, the wait time will be inserted at the end of paintGL
            // in order to achieve precise playback
            emit nextFrame(fps, &m_timer, targetInstant);

            // Playing on Viewer / Combo Viewer will advance the current frame.
            // Calling qApp->processEvents() on drawing frame causes repaint of other
            // panels which slows playback. Therefore in Viewer / Combo Viewer panels
            // it just calls update() and necessary wait will be inserted here.
            qint64 currentInstant = m_timer.nsecsElapsed();
            while (currentInstant < targetInstant) {
                currentInstant = m_timer.nsecsElapsed();
            }

        }

        //-------- Each nextFrame() blocks until the frame has been shown ---------

        // accumurate error and slightly adjust waiting time for subsequent frames
        qint64 delay = m_timer.nsecsElapsed() - emissionInstant - targetFrameTime;
        // just ignore a large error
        if (delay < targetFrameTime) shortTermDelayAdjuster += delay;

        ++playedFramesCount;
    }
    emit(playbackAborted());
    m_abort = false;
    m_timer.invalidate();
}