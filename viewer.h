#ifndef VIEWER_H
#define VIEWER_H

#include "drawing.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <memory> 
class Director;
class StopMotion;

class Viewer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    Viewer(QWidget* parent = nullptr,
        Qt::WindowFlags f = Qt::WindowFlags());
    ~Viewer();

    double m_scale = 1.0;
    double m_maxScale = 1.0;
    QImage m_currentImage;
    QImage m_lineUpImage;
    QImage m_testImage;
    bool m_showTestShot = false;
    QString m_testInfo;

    void setPaths();
    void setBackgroundColor(QColor color) { m_backgroundColor = color; }
    void setOverlayOpacity(int opacity) { m_overlayOpacity = opacity; }
    void setOverlayRatio(double ratio) { m_overlayRatio = ratio; }

private:
    StopMotion *m_stopMotion;
    Director* m_director;
    
    QImage m_liveViewImage;
    int m_imgHeight = 1080;
    int m_imgWidth = 1920;
    
    void paintEvent(QPaintEvent*);
    void wheelEvent(QWheelEvent* event);
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event) override;
    bool event(QEvent* event) override;

    bool m_scaleSet = false;
    void getScale();    
    
    void translateClickPos();
    QPointF translatePointToScreen(QPointF point, bool useImage = false);
    void translateClickPointToImage();
    QPointF m_clickPos = QPoint(200.0, 200.0);
    QPointF m_realClickPos = QPoint(200.0, 200.0);
    QPoint m_lastPos;
    QPointF m_viewOffset = QPointF(0.0, 0.0);
    Drawing m_drawing;
    bool m_mouseDown = false;
    bool m_dragging = false;

    QPointF m_firstClickPos = QPointF(-1.0, -1.0);
    QPointF m_lastClickPos = QPointF(-1.0, -1.0);
    QRectF getShapeRect();
    QRectF getScaledShapeRect();
    
    int m_highlighPath = -1;

    bool m_showLiveViewTemp = false;
    bool m_setLiveView = false;
    bool m_changingTop, m_changingBottom, m_changingLeft, m_changingRight, m_shift;

    QColor m_backgroundColor = Qt::black;
    int m_overlayOpacity = 100;
    double m_overlayRatio = 2.39;
    bool m_liveViewShownOnce = false;
    QRectF m_topLeft, m_topCenter, m_topRight, m_left, m_right, m_bottomLeft, m_bottomCenter, m_bottomRight;

public slots:
    void getLiveViewImage();
    void liveViewStopped();
    void fitViewer(bool ignore= false);
    void onPathSelectedSignal(int);
};
#endif // VIEWER_H
