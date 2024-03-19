#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QImage>
#include <QEvent>
#include <QMouseEvent>
#include <memory>

class StopMotion;

class Stroke : public QObject
{
    Q_OBJECT

public:
    Stroke();
    Stroke(const Stroke& stroke);
    Stroke operator = (const Stroke& other) {
        if (this == &other)
            return *this;
        m_bezierPoints = other.m_bezierPoints;
        m_steps = other.m_steps;
        m_color = other.m_color;
        m_thickness = other.m_thickness;
        return *this;
    }

    ~Stroke();

    std::vector<QPointF> getBezierPoints() { return m_bezierPoints; };
    void setPoints(std::vector<QPointF> points);
    QColor getColor() { return m_color; }
    void setColor(QColor color) { m_color = color; }
    int getSteps() { return m_steps; }
    void setSteps(int steps) { m_steps = steps; }
    int getThickness() { return m_thickness; }
    void setThickness(int thickness) { m_thickness = thickness; }

private:
    std::vector<QPointF> m_bezierPoints;
    int m_steps = 10;
    QColor m_color = Qt::red;
    int m_thickness = 5;

};


#endif // STROKE_H
