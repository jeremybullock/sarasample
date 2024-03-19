#ifndef MOTIONPATH_H
#define MOTIONPATH_H

#include <QObject>
#include <QImage>
#include <memory>
#include <QPainterPath>

class StopMotion;

class MotionPath : public QObject
{
    Q_OBJECT

public:
    enum TYPE { PATH, LINE, CIRCLE, RECTANGLE, TEXT };
    MotionPath();
    MotionPath(const MotionPath& MotionPath);
    MotionPath operator = (const MotionPath& other) {
        if (this == &other)
            return *this;

        m_points = other.m_points;
        m_interpolationPoints = other.m_interpolationPoints;
        m_color = other.m_color;
        m_width = other.m_width;
        m_text = other.m_text;
        m_stepCount = other.m_stepCount;
        m_path = other.m_path;
        m_type = other.m_type;
        m_rawPoints = other.m_rawPoints;
        m_stepPoints = other.m_stepPoints;
        m_originalPath = other.m_originalPath;
        m_currentStep = other.m_currentStep;
        return *this;
    }
    ~MotionPath();
    QList<QPointF> getPoints() { return m_points; }
    void setPoints(QList<QPointF> points) { m_points = points; }
    
    QString getText() { return m_text; }
    void setText(QString name) { m_text = name; }

    void setRawPoints(std::vector<QPointF> points) { m_rawPoints = points; }
    std::vector<QPointF> getRawPoints() { return m_rawPoints; }
    
    QList<QPointF> getIterpolationPoints() { return m_interpolationPoints; }
    void setInterpolationPoints(QList<QPointF> points) { m_interpolationPoints = points; }
    
    QColor getColor() { return m_color; }
    void setColor(QColor color) { m_color = color; }
    
    int getWidth() { return m_width; }
    void setWidth(int width) {m_width = width; }
    
    int getStepCount() { return m_stepCount; }
    void setStepCount(int count) {m_stepCount = count; }
    
    int getCurrentStep() { return m_currentStep; }
    void setCurrentStep(int step) { m_currentStep = step; }
    
    QPainterPath getInterpolationPainterPath();
    
    QPainterPath getPainterPath() { return m_path; }
    void setPainterPath(QPainterPath path) { m_path = path; }
    
    QPainterPath getOriginalPainterPath() { return m_originalPath; }
    void setOriginalPainterPath(QPainterPath path) { m_originalPath = path; }
    
    std::vector<std::pair<QPointF, double>> getStepPoints() { return m_stepPoints; }
    void setStepPoints(std::vector<std::pair<QPointF, double>> stepPoints) { m_stepPoints = stepPoints; }
    
    int getType() { return m_type; }
    void setType(int type) { m_type = type; }

    void move(QPointF);
    void scaleMotionPath(bool top, bool bottom, bool left, bool right, QPointF newPos, bool shift);
    void resetEdit();

private:
    int m_type = PATH;
    QColor m_color = Qt::red;
    int m_stepCount = 0;
    QString m_text;
    int m_width;
    int m_currentStep;
    QList<QPointF> m_interpolationPoints;
    QPainterPath m_path;
    QPainterPath m_originalPath;
    std::vector<std::pair<QPointF, double>> m_stepPoints;

    // these are what needs to be moved in a move event
    std::vector<QPointF> m_rawPoints;
    QList<QPointF> m_points;

    // temporary variables
    int m_leftPoint = -1;
    int m_rightPoint = -1;
    int m_topPoint = -1;
    int m_bottomPoint = -1;
    std::vector<QPointF> m_tempRawPoints;
    QList<QPointF> m_tempPoints;
    double m_tempRatio = -1.0;
    double m_farLeft = 0.0;
    double m_farRight = 0.0;
    double m_farTop = 0.0;
    double m_farBottom = 0.0;
};
#endif // MOTIONPATH_H
