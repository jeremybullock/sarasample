#ifndef DRAWING_H
#define DRAWING_H

#include <QObject>
#include <QImage>
#include <QEvent>
#include <QMouseEvent>
#include <memory>

class StopMotion;

//************************************************************************
//    Smooth Stroke declaration
//    Brush stroke smoothing buffer.
//************************************************************************
class SmoothStroke {
public:
    SmoothStroke() {}
    ~SmoothStroke() {}

    // begin stroke
    // smooth is smooth strength, from 0 to 100
    void beginStroke(int smooth);
    // add stroke point
    void addPoint(const QPointF& point);
    // end stroke
    void endStroke();
    // Get generated stroke points which has been smoothed.
    // Both addPoint() and endStroke() generate new smoothed points.
    // This method will removed generated points
    void getSmoothPoints(std::vector<QPointF>& smoothPoints);
    // Remove all points - used for straight lines
    void clearPoints();

private:
    void generatePoints();

private:
    int m_smooth;
    int m_outputIndex;
    int m_readIndex;
    std::vector<QPointF> m_rawPoints;
    std::vector<QPointF> m_outputPoints;
    int m_resampledIndex;
    std::vector<QPointF> m_resampledPoints;
};


class Drawing : public QObject
{
    Q_OBJECT

public:
    Drawing();
    Drawing(const Drawing& stroke);
    Drawing operator = (const Drawing& other) {
        if (this == &other)
            return *this;
        m_points = other.m_points;
        return *this;
    }

    ~Drawing();

    std::vector<QPointF> getPoints() { return m_points; };
    //std::vector<QPointF> getRawPoints() { return m_rawPoints; };
    QList<QPointF> getBezierPoints() { return m_bezierPoints; };
    void leftButtonDown(const QPointF& pos, const QMouseEvent& e);
    void leftButtonDrag(const QPointF& pos, const QMouseEvent& e);
    void leftButtonUp(const QPointF& pos, const QMouseEvent& e);
    void clearPoints();

private:
    std::vector<QPointF> m_points;
    //std::vector<QPointF> m_rawPoints;
    QList<QPointF> m_bezierPoints;
    //QPointF m_firstPoint, m_lastPoint;
    //QPointF m_p0, m_p1;

    SmoothStroke m_smoothStroke;
    double m_smooth = 4.0;
    void addTrackPoint(const QPointF& point,
        double pixelSize2);
    void addPoint(const QPointF& point, double pixelSize2);
    void flushTrackPoint();
};


#endif // DRAWING_H
