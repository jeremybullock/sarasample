#pragma once

#ifndef MOTIONPATHPANEL_H
#define MOTIONPATHPANEL_H

#include "motionpath.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QImage>
#include <QThread>

class Director;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QFrame;
class QToolBar;
class QSlider;
class QComboBox;
class GraphWidget;
class QPushButton;
class QLabel;

//=============================================================================
// ClickablePathLabel
//-----------------------------------------------------------------------------

class ClickablePathLabel : public QLabel {
  Q_OBJECT

protected:
  void mouseReleaseEvent(QMouseEvent*) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  //void enterEvent(QEvent*) override;
  //void leaveEvent(QEvent*) override;

public:
  ClickablePathLabel(const QString& text, QWidget* parent = nullptr,
                     Qt::WindowFlags f = Qt::WindowFlags());
  ~ClickablePathLabel();
  void setSelected();
  void clearSelected();

signals:
  void onMouseRelease(QMouseEvent* event);
  void doubleClicked();
};

//-----------------------------------------------------------------------------

class MotionPathPlaybackExecutor final : public QThread {
    Q_OBJECT

    int m_fps;
    bool m_abort;
    QElapsedTimer m_timer;

public:
    MotionPathPlaybackExecutor();

    void resetFps(int fps);

    void run() override;
    void abort() { m_abort = true; }

    bool isAborted() { return m_abort; }
    void emitNextFrame(int fps) { emit nextFrame(fps, nullptr, 0); }

signals:
    void nextFrame(
        int fps, QElapsedTimer* timer,
        qint64 targetInstant);  // Must be connect with
                                // Qt::BlockingQueuedConnection connection type.
    void playbackAborted();
};

//=============================================================================
// MotionPathPanel
//-----------------------------------------------------------------------------

class MotionPathPanel final : public QWidget {
  Q_OBJECT

  Q_PROPERTY(QColor SelectedColor READ getSelectedColor WRITE setSelectedColor
                 DESIGNABLE true)
  QColor m_selectedColor;
  QColor getSelectedColor() const { return m_selectedColor; }
  void setSelectedColor(const QColor& color) { m_selectedColor = color; }

  Q_PROPERTY(
      QColor HoverColor READ getHoverColor WRITE setHoverColor DESIGNABLE true)
  QColor m_hoverColor;
  QColor getHoverColor() const { return m_hoverColor; }
  void setHoverColor(const QColor& color) { m_hoverColor = color; }
  std::vector<QHBoxLayout*> m_nameLayouts;

  QHBoxLayout* m_toolLayout;
  //QVBoxLayout* m_controlsLayout;
  QGridLayout* m_outsideLayout;
  QToolBar* m_toolbar;
  QToolBar* m_playToolbar;
  QFrame* m_graphFrame;
  QPushButton *m_colorButton;
  bool m_looping = false;
  QColor m_currentColor = Qt::red;

  std::vector<std::vector<QPointF>> m_splines;
  MotionPath m_currentPath;
  MotionPath m_placeHolderPath;
  GraphWidget* m_graphArea;
  std::vector<ClickablePathLabel*> m_pathLabels;

  MotionPathPlaybackExecutor m_playbackExecutor;

public:
  MotionPathPanel(QWidget* parent = 0);
  ~MotionPathPanel();

protected:
  void clearPathsLayout();
  void newPath();
  void showEvent(QShowEvent*) override;
  bool m_showLiveViewTemp = false;
  bool m_setLiveView = false;
  bool m_playReversed = false;

  QLabel* m_widthLabel, * m_stepsLabel, *m_colorLabel;
  QSlider *m_stepsSlider, *m_widthSlider;
  Director* m_director;

protected slots:
  //void onNextFrame(int);
  void onNextFrame(
      int fps, QElapsedTimer* timer,
      qint64 targetInstant);
  void stopPlayback();
  //void onNewPathSignal();
  void onPathSelectedSignal(int);
  void setColor();
  // public slots:
};

#endif  // MOTIONPATHPANEL_H
