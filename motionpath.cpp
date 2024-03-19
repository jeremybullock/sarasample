#include "motionpath.h"
#include <QPainterPath>
#include <QVector2D>


void truncateSpeeds(double aFrame, double bFrame, QVector2D& aSpeedTrunc,
    QVector2D& bSpeedTrunc) {
    double deltaX = bFrame - aFrame;
    if (aSpeedTrunc.x() < 0) aSpeedTrunc.setX(0);
    if (bSpeedTrunc.x() > 0) bSpeedTrunc.setX(0);

    if (aFrame + aSpeedTrunc.x() > bFrame) {
        if (aSpeedTrunc.x() != 0) {
            aSpeedTrunc = aSpeedTrunc * (deltaX / aSpeedTrunc.x());
        }
    }

    if (bFrame + bSpeedTrunc.x() < aFrame) {
        if (bSpeedTrunc.x() != 0) {
            bSpeedTrunc = -bSpeedTrunc * (deltaX / bSpeedTrunc.x());
        }
    }
}

//-----------------------------------------------------------------------------

QList<QPointF> getIntersectedPoint(QRectF rect, QPainterPath path) {
    int y0 = rect.top();
    int y1 = rect.bottom();
    QList<QPointF> points;
    double g = 1.0 / 256.0;
    QPointF prec = path.pointAtPercent(0);
    QPointF point = path.pointAtPercent(g);
    int j = 0;
    for (j = 2; j < 256; j++) {
        QPointF next = path.pointAtPercent(double(j) * g);
        if (prec.y() >= y0 && prec.y() <= y1) {
            if (point.y() < y0)
                points.push_back(QPointF(prec.x(), y0));
            else if (point.y() > y1)
                points.push_back(QPointF(prec.x(), y1));
        }
        if (next.y() >= y0 && next.y() <= y1) {
            if (point.y() < y0)
                points.push_back(QPointF(next.x(), y0));
            else if (point.y() > y1)
                points.push_back(QPointF(next.x(), y1));
        }
        prec = point;
        point = next;
    }
    return points;
}

//---------------------------------------------------------

MotionPath::MotionPath() {
    m_interpolationPoints.clear();
    m_interpolationPoints.push_back(QPointF(-40, 0));
    m_interpolationPoints.push_back(QPointF(-20, 0));
    m_interpolationPoints.push_back(QPointF(-20, 0));
    m_interpolationPoints.push_back(QPointF(0, 0));
    m_interpolationPoints.push_back(QPointF(65, 65));

    m_interpolationPoints.push_back(QPointF(935, 935));
    m_interpolationPoints.push_back(QPointF(1000, 1000));
    m_interpolationPoints.push_back(QPointF(1020, 1000));
    m_interpolationPoints.push_back(QPointF(1020, 1000));
    m_interpolationPoints.push_back(QPointF(1040, 1000));
    m_color = Qt::red;
    m_width = 1;
}

MotionPath::MotionPath(const MotionPath& MotionPath) {
    m_points = MotionPath.m_points;
    m_interpolationPoints = MotionPath.m_interpolationPoints;
    m_color = MotionPath.m_color;
    m_width = MotionPath.m_width;
    m_text = MotionPath.m_text;
    m_stepCount = MotionPath.m_stepCount;
    m_type = MotionPath.m_type;
    m_rawPoints = MotionPath.m_rawPoints;
    m_stepPoints = MotionPath.m_stepPoints;
    m_path = MotionPath.m_path;
    m_originalPath = MotionPath.m_originalPath;
    m_currentStep = MotionPath.m_currentStep;
    //return *this;
}

MotionPath::~MotionPath() {};

//-----------------------------------------------------------------------------
// this is not used anywhere.
QPainterPath MotionPath::getInterpolationPainterPath() {
    int pointCount = m_interpolationPoints.size();
    if (pointCount == 0) return QPainterPath();

    QPointF p0 = m_interpolationPoints.at(0);
    QPainterPath path(p0);
    if (m_points.size() < 6) return path;
    int i;
    for (i = 1; i < pointCount; i++) {
        QPointF p1 = m_interpolationPoints.at(i);
        QPointF p2 = m_interpolationPoints.at(++i);
        QPointF p3 = m_interpolationPoints.at(++i);
        path.moveTo(p0);
        if (true) {
            // truncate speed
            QVector2D aSpeed(p1 - p0);
            QVector2D bSpeed(p2 - p3);
            truncateSpeeds(p0.x(), p3.x(), aSpeed, bSpeed);
            path.cubicTo(p0 + aSpeed.toPointF(), p3 + bSpeed.toPointF(), p3);
        }
        else
            path.lineTo(p3);
        p0 = p3;
    }

    QRectF rect(0, 0, 1000, 1000);
    QRectF r = path.boundingRect();
    if (!rect.contains(QRect(rect.left(), r.top(), rect.width(), r.height()))) {
        QList<QPointF> points = getIntersectedPoint(rect, path);

        int j = 0;
        for (j = 0; j < points.size(); j++) {
            QPointF p0 = points.at(j);
            QPointF p1 = points.at(++j);
            QPainterPath line(p0);
            line.lineTo(p1);
            path.addPath(line);
        }
    }

    return path;
}

//-----------------------------------------------------------------------------

void MotionPath::move(QPointF change) {
    for (int i = 0; i < m_points.size(); i++) {
        m_points[i] += change;
    }
    for (int i = 0; i < m_rawPoints.size(); i++) {
        m_rawPoints[i] += change;
    }
}

//-----------------------------------------------------------------------------

void MotionPath::scaleMotionPath(bool top, bool bottom, bool left, bool right, QPointF newPos, bool shift) {
    if (m_tempPoints.isEmpty()) {
        m_tempPoints = m_points;
        m_tempRawPoints = m_rawPoints;
    }

    if (m_type == RECTANGLE || m_type == CIRCLE) {
        double width = m_points[1].x() - m_points[0].x();
        double height = m_points[1].y() - m_points[0].y();
        if (m_tempRatio == -1.0) {
            m_tempRatio = width / height;
        }
        if (top && left) {
            m_points[0].setX(newPos.x());
            if (shift) {
                double newHeight = width / m_tempRatio;
                m_points[0].setY(m_tempPoints[1].y() - newHeight);
            }
            else m_points[0].setY(newPos.y());

        }
        else if (top && right) {
            m_points[1].setX(newPos.x());
            if (shift) {
                double newHeight = width / m_tempRatio;
                m_points[0].setY(m_tempPoints[1].y() - newHeight);
            }
            else m_points[0].setY(newPos.y());

        }
        else if (bottom && left) {
            m_points[0].setX(newPos.x());
            if (shift) {
                double newHeight = width / m_tempRatio;
                m_points[1].setY(m_tempPoints[0].y() + newHeight);
            }
            else m_points[1].setY(newPos.y());

        }
        else if (bottom && right) {
            m_points[1].setX(newPos.x());
            if (shift) {
                double newHeight = width / m_tempRatio;
                m_points[1].setY(m_tempPoints[0].y() + newHeight);
            }
            else m_points[1].setY(newPos.y());

        }
        else {
            if (top) {
                m_points[0].setY(newPos.y());
            }
            if (bottom) {
                m_points[1].setY(newPos.y());
            }
            if (left) {
                m_points[0].setX(newPos.x());
            }
            if (right) {
                m_points[1].setX(newPos.x());
            }
        }
    }
    else if (m_type == LINE) {
        if (m_leftPoint == -1) {
            m_leftPoint = m_points[0].x() < m_points[1].x() ? 0 : 1;
            m_rightPoint = m_points[0].x() > m_points[1].x() ? 0 : 1;
            m_topPoint = m_points[0].y() < m_points[1].y() ? 0 : 1;
            m_bottomPoint = m_points[0].y() > m_points[1].y() ? 0 : 1;
        }

        if (top) {
            m_points[m_topPoint].setY(newPos.y());
        }
        if (bottom) {
            m_points[m_bottomPoint].setY(newPos.y());
        }
        if (left) {
            m_points[m_leftPoint].setX(newPos.x());
        }
        if (right) {
            m_points[m_rightPoint].setX(newPos.x());
        }

    }
    else if (m_type == PATH) {
        double width;
        double height;

        if (m_tempRatio == -1.0) {

            double farLeft = m_rawPoints[0].x();
            double farRight = m_rawPoints[0].x();
            double farTop = m_rawPoints[0].y();
            double farBottom = m_rawPoints[0].y();

            for (int i = 1; i < m_rawPoints.size(); i++) {
                if (m_rawPoints[i].x() < farLeft) {
                    farLeft = m_rawPoints[i].x();
                }
                if (m_rawPoints[i].x() > farRight) {
                    farRight = m_rawPoints[i].x();
                }
                if (m_rawPoints[i].y() < farTop) {
                    farTop = m_rawPoints[i].y();
                }
                if (m_rawPoints[i].y() > farBottom) {
                    farBottom = m_rawPoints[i].y();
                }
            }

            height = farBottom - farTop;
            width = farRight - farLeft;

            m_tempRatio = width / height;
            m_tempPoints = m_points;
            m_tempRawPoints = m_rawPoints;
            m_farLeft = farLeft;
            m_farRight = farRight;
            m_farTop = farTop;
            m_farBottom = farBottom;
        }

        height = m_farBottom - m_farTop;
        width = m_farRight - m_farLeft;
        if (top) {
            double newHeight = m_farBottom - newPos.y();
            for (int i = 0; i < m_rawPoints.size(); i++) {
                double originalDistance = m_farBottom - m_tempRawPoints[i].y();
                if (originalDistance > 0) {
                    double percentage = originalDistance / height;
                    double newY = m_farBottom - (newHeight * percentage);
                    m_rawPoints.at(i).setY(newY);
                }
            }

            for (int i = 0; i < m_points.size(); i++) {
                double originalDistance = m_farBottom - m_tempPoints[i].y();
                if (originalDistance > 0) {
                    double percentage = originalDistance / height;
                    double newY = m_farBottom - (newHeight * percentage);
                    m_points[i].setY(newY);
                }
            }
        }

        if (bottom) {
            double newHeight = newPos.y() - m_farTop;
            for (int i = 0; i < m_rawPoints.size(); i++) {
                double originalDistance = m_tempRawPoints[i].y() - m_farTop;
                if (originalDistance > 0) {
                    double percentage = originalDistance / height;
                    double newY = m_farTop + (newHeight * percentage);
                    m_rawPoints.at(i).setY(newY);
                }
            }

            for (int i = 0; i < m_points.size(); i++) {
                double originalDistance = m_tempPoints[i].y() - m_farTop;
                if (originalDistance > 0) {
                    double percentage = originalDistance / height;
                    double newY = m_farTop + (newHeight * percentage);
                    m_points[i].setY(newY);
                }
            }
        }

        if (left) {
            double newWidth = m_farRight - newPos.x();
            for (int i = 0; i < m_rawPoints.size(); i++) {
                double originalDistance = m_farRight - m_tempRawPoints[i].x();
                if (originalDistance > 0) {
                    double percentage = originalDistance / width;
                    double newX = m_farRight - (newWidth * percentage);
                    m_rawPoints.at(i).setX(newX);
                }
            }

            for (int i = 0; i < m_points.size(); i++) {
                double originalDistance = m_farRight - m_tempPoints[i].x();
                if (originalDistance > 0) {
                    double percentage = originalDistance / width;
                    double newX = m_farRight - (newWidth * percentage);
                    m_points[i].setX(newX);
                }
            }
        }

        if (right) {
            double newWidth = newPos.x() - m_farLeft;
            for (int i = 0; i < m_rawPoints.size(); i++) {
                double originalDistance = m_tempRawPoints[i].x() - m_farLeft;
                if (originalDistance > 0) {
                    double percentage = originalDistance / width;
                    double newX = m_farLeft + (newWidth * percentage);
                    m_rawPoints.at(i).setX(newX);
                }
            }

            for (int i = 0; i < m_points.size(); i++) {
                double originalDistance = m_tempPoints[i].x() - m_farLeft;
                if (originalDistance > 0) {
                    double percentage = originalDistance / width;
                    double newX = m_farLeft + (newWidth * percentage);
                    m_points[i].setX(newX);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

void MotionPath::resetEdit() {
    if (m_type == RECTANGLE || m_type == CIRCLE) {
        int leftPoint = m_points[0].x() < m_points[1].x() ? 0 : 1;
        int rightPoint = m_points[0].x() > m_points[1].x() ? 0 : 1;
        int topPoint = m_points[0].y() < m_points[1].y() ? 0 : 1;
        int bottomPoint = m_points[0].y() > m_points[1].y() ? 0 : 1;

        QPointF topLeft = QPointF(m_points[leftPoint].x(), m_points[topPoint].y());
        QPointF bottomRight = QPointF(m_points[rightPoint].x(), m_points[bottomPoint].y());
        m_points[0] = topLeft;
        m_points[1] = bottomRight;
    }

    m_leftPoint = -1;
    m_rightPoint = -1;
    m_topPoint = -1;
    m_bottomPoint = -1;
    m_tempPoints.clear();
    m_tempRawPoints.clear();
    m_tempRatio = -1.0;
}