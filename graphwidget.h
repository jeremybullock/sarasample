#pragma once

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QImage>

class QFrame;
class TThickPoint;
class Director;

//=============================================================================
// GraphArea
//-----------------------------------------------------------------------------

class GraphWidget : public QWidget {
  Q_OBJECT

  Q_PROPERTY(QColor SplineColor READ getSplineColor WRITE setSplineColor
                 DESIGNABLE true)
  QColor m_splineColor;
  QColor getSplineColor() const { return m_splineColor; }
  void setSplineColor(const QColor& color) { m_splineColor = color; }

  Q_PROPERTY(
      QColor GraphColor READ getGraphColor WRITE setGraphColor DESIGNABLE true)
  QColor m_graphColor;
  QColor getGraphColor() const { return m_graphColor; }
  void setGraphColor(const QColor& color) { m_graphColor = color; }

  Q_PROPERTY(QColor NonSelectedPointColor READ getNonSelectedPointColor WRITE
                 setNonSelectedPointColor DESIGNABLE true)
  QColor m_nonSelectedPointColor;
  QColor getNonSelectedPointColor() const { return m_nonSelectedPointColor; }
  void setNonSelectedPointColor(const QColor& color) {
    m_nonSelectedPointColor = color;
  }

  Q_PROPERTY(QColor SelectedPointColor READ getSelectedPointColor WRITE
                 setSelectedPointColor DESIGNABLE true)
  QColor m_selectedPointColor;
  QColor getSelectedPointColor() const { return m_selectedPointColor; }
  void setSelectedPointColor(const QColor& color) {
    m_selectedPointColor = color;
  }

  QList<QPointF> m_points;
  int m_currentControlPointIndex = -1;

  Qt::MouseButton m_mouseButton;

  // int m_maxHeight;
  // int m_maxWidth;

  int m_LeftRightMargin;
  int m_TopMargin;
  int m_BottomMargin;

  bool m_isLinear;
  bool m_lockExtremePoints = true;
  bool m_constrainToBounds = true;

  QPointF m_preMousePos;

  bool m_isEnlarged;

  double m_maxXValue = 255.0;
  double m_maxYValue = 255.0;
  double m_cpMargin  = 20.0;

public:
  explicit GraphWidget(QWidget* parent = nullptr);
  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;
  void setMaxXValue(int x) { m_maxXValue = x; }
  void setMaxYValue(int y) { m_maxYValue = y; }

  void setPoints(QList<QPointF> points);
  void clearPoints() { m_points.clear(); }
  QList<QPointF> getPoints();

  int getCurrentControlPointIndex() { return m_currentControlPointIndex; };
  void setCurrentControlPointIndex(int index) {
    m_currentControlPointIndex = index;
  };

  bool eventFilter(QObject* object, QEvent* event) override;

  void setFirstLastXPosition(std::pair<double, double> values, bool isDragging);

  void setLinear(bool isLinear);
  void moveCurrentControlPoint(QPointF delta);

  // void setEnlarged(bool isEnlarged);

  QPointF convertPointToLocal(QPointF point);
  QPointF convertPointFromLocal(QPointF point);
  bool removeCurrentControlPoint();

protected:
  QPointF viewToStrokePoint(const QPointF& p);
  QPointF getInvertedPoint(QPointF p);
  int getClosestPointIndex(QPointF& pos, double& minDistance2);

  bool isCentralControlPoint(const int index) const { return index % 3 == 0; }
  bool isLeftControlPoint(const int index) const { return index % 3 == 2; }
  bool isRightControlPoint(const int index) const { return index % 3 == 1; }

  void setPoint(int index, const QPointF point);
  void movePoint(int index, const QPointF delta);
  QPointF checkPoint(const QPointF p);

  QPointF getVisibleHandlePos(int index);

  void moveCentralControlPoint(int index, QPointF delta);
  bool middlePointSelected();

  void addControlPoint(double percent);

  bool removeControlPoint(int index);

  void selectNextControlPoint();
  void selectPreviousControlPoint();

  QPainterPath getPainterPath();

  void paintEvent(QPaintEvent*) override;
  void mouseMoveEvent(QMouseEvent*) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  //void keyPressEvent(QKeyEvent* e) override;

  void focusInEvent(QFocusEvent* fe) override;
  void focusOutEvent(QFocusEvent* fe) override;

  Director* m_director;

signals:
  void focusOut();
  void controlPointChanged(bool isDragging);
  void controlPointAdded(int index);
  void controlPointRemoved(int index);

  void firstLastXPostionChanged(double, double);
  void updateCurrentPosition(int, QPointF);
};

#endif  // GRAPHWIDGET_H
