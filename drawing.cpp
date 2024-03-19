#include "drawing.h"
#include "stopmotion/stopmotion.h"

#include "thirdparty/curve-fit-nd/curve_fit_nd.h"


double norm2(const QPointF& p) { return p.x() * p.x() + p.y() * p.y(); }
double norm(const QPointF& p) { return std::sqrt(norm2(p)); }

double tdistance(const QPointF& p1, const QPointF& p2) {
    return norm(p2 - p1);
}

double tdistance2(const QPointF& p1, const QPointF& p2) {
    return norm2(p2 - p1);
}

QRectF addRect(QRectF rect1, QRectF rect2) {
    double x = std::min(rect1.topLeft().x(), rect2.topLeft().x());
    double y = std::min(rect1.topLeft().y(), rect2.topLeft().y());
    double width = std::max(rect1.width(), rect2.width());
    double height = std::max(rect1.height(), rect2.height());
    return QRectF(x, y, width, height);
}

Drawing::Drawing() {
}

Drawing::Drawing(const Drawing& drawing) {
    m_points = drawing.m_points;
}

Drawing::~Drawing() {};

void Drawing::clearPoints() {
    m_bezierPoints.clear();
    m_points.clear();
    //m_rawPoints.clear();
}

//--------------------------------------------------------------------------------------------------

void Drawing::leftButtonDown(const QPointF& pos,
    const QMouseEvent& e) {

    m_points.clear();
    //m_rawPoints.clear();
    m_bezierPoints.clear();
    double thickness = 0.0; 

    m_smoothStroke.beginStroke(m_smooth);
    //m_rawPoints.push_back(pos);
    addTrackPoint(pos, 1.0);
}

//-------------------------------------------------------------------------------------------------------------

void Drawing::leftButtonDrag(const QPointF& pos,
    const QMouseEvent& e) {
    //m_rawPoints.push_back(pos);
    addTrackPoint(pos, 1.0);
}

//---------------------------------------------------------------------------------------------------------------

void Drawing::leftButtonUp(const QPointF& pos,
    const QMouseEvent& e) {
    //m_rawPoints.push_back(pos);
    flushTrackPoint();

    std::vector<double> rawestPoints;
    //for (auto point : m_rawPoints) {
    for (auto point : m_points) {
        rawestPoints.push_back(point.x());
        rawestPoints.push_back(point.y());
    }

    int rawPointsSize = m_points.size();
    //int rawPointsSize = m_rawPoints.size();


    double* pointsPointer = &rawestPoints[0];

    unsigned int* r_corners = NULL;
    unsigned int r_corners_len = 0;

    double* cubic_spline = NULL;
    unsigned int cubic_spline_len = 0;
    unsigned int* r_cubic_orig_index;
    unsigned int* corners_index = NULL;
    unsigned int  corners_index_len = 0;

    if (cubic_spline_len > 0) {
        m_bezierPoints.clear();
        int i = 0;
        for (int i = 0; i < cubic_spline_len * 6;) {
            m_bezierPoints.push_back(QPointF(cubic_spline[i], cubic_spline[i+1]));
            m_bezierPoints.push_back(QPointF(cubic_spline[i+2], cubic_spline[i+3]));
            m_bezierPoints.push_back(QPointF(cubic_spline[i+4], cubic_spline[i+5]));
            i += 6;
        }
    }
    int bezSize = m_bezierPoints.size();
}

//--------------------------------------------------------------------------------------------------

void Drawing::addTrackPoint(const QPointF& point,
    double pixelSize2) {
    m_smoothStroke.addPoint(point);
    std::vector<QPointF> pts;
    m_smoothStroke.getSmoothPoints(pts);
    
    for (size_t i = 0; i < pts.size(); ++i) {
        addPoint(pts[i], pixelSize2);
    }
}

//-------------------------------------------------------------------

void Drawing::addPoint(const QPointF& point, double pixelSize2) {
    if (m_points.empty()) {
        double x = point.x(), y = point.y(), d = 3;
        m_points.push_back(point);
    }
    else {
        QPointF lastPoint = m_points.back();
        if (tdistance2(lastPoint, point) >= 4 * pixelSize2) {
            m_points.push_back(point);
        }
    }
}

//--------------------------------------------------------------------------------------------------

void Drawing::flushTrackPoint() {
    m_smoothStroke.endStroke();
    std::vector<QPointF> pts;
    m_smoothStroke.getSmoothPoints(pts);
    double pixelSize2 = 1.0;
    for (size_t i = 0; i < pts.size(); ++i) {
        addPoint(pts[i], pixelSize2);
    }
}
    
//--------------------------------------------------------------------------------------------------

static void CatmullRomInterpolate(const QPointF& P0, const QPointF& P1,
    const QPointF& P2, const QPointF& P3,
    int samples,
    std::vector<QPointF>& points) {
    double x0 = P1.x();
    double x1 = (-P0.x() + P2.x()) * 0.5f;
    double x2 = P0.x() - 2.5f * P1.x() + 2.0f * P2.x() - 0.5f * P3.x();
    double x3 = -0.5f * P0.x() + 1.5f * P1.x() - 1.5f * P2.x() + 0.5f * P3.x();

    double y0 = P1.y();
    double y1 = (-P0.y() + P2.y()) * 0.5f;
    double y2 = P0.y() - 2.5f * P1.y() + 2.0f * P2.y() - 0.5f * P3.y();
    double y3 = -0.5f * P0.y() + 1.5f * P1.y() - 1.5f * P2.y() + 0.5f * P3.y();

    for (int i = 1; i <= samples; ++i) {
        double t = i / (double)(samples + 1);
        double t2 = t * t;
        double t3 = t2 * t;
        QPointF p;
        p.setX(x0 + x1 * t + x2 * t2 + x3 * t3);
        p.setY(y0 + y1 * t + y2 * t2 + y3 * t3);
        points.push_back(p);
    }
}

//--------------------------------------------------------------------------------------------------

static void Smooth(std::vector<QPointF>& points, const int radius,
    const int readIndex, const int level) {
    int n = (int)points.size();
    if (radius < 1 || n < 3) {
        return;
    }

    std::vector<QPointF> result;

    float d = 1.0f / (radius * 2 + 1);

    int endSamples = 10;
    int startId = std::max(readIndex - endSamples * 3 - radius * level, 1);

    for (int i = startId; i < n - 1; ++i) {
        int lower = i - radius;
        int upper = i + radius;

        QPointF total;
        total.setX(0);
        total.setY(0);

        for (int j = lower; j <= upper; ++j) {
            int idx = j;
            if (idx < 0) {
                idx = 0;
            }
            else if (idx >= n) {
                idx = n - 1;
            }
            total.setX( total.x() + points[idx].x());
            total.setY( total.y() + points[idx].y());
        }

        total.setX(total.x() * d);
        total.setY(total.y() * d);
        result.push_back(total);
    }

    auto result_itr = result.begin();
    for (int i = startId; i < n - 1; ++i, ++result_itr) {
        points[i].setX((*result_itr).x());
        points[i].setY((*result_itr).y());
    }

    if (points.size() >= 3) {
        std::vector<QPointF> pts;
        CatmullRomInterpolate(points[0], points[0], points[1], points[2],
            endSamples, pts);
        std::vector<QPointF>::iterator it = points.begin() + 1;
        points.insert(it, pts.begin(), pts.end());

        pts.clear();
        CatmullRomInterpolate(points[n - 3], points[n - 2], points[n - 1],
            points[n - 1], 10, pts);
        it = points.begin();
        it += n - 1;
        points.insert(it, pts.begin(), pts.end());
    }
}

//--------------------------------------------------------------------------------------------------

void SmoothStroke::beginStroke(int smooth) {
    m_smooth = smooth;
    m_outputIndex = 0;
    m_readIndex = -1;
    m_rawPoints.clear();
    m_outputPoints.clear();
    m_resampledIndex = 0;
    m_resampledPoints.clear();
}

//--------------------------------------------------------------------------------------------------

void SmoothStroke::addPoint(const QPointF& point) {
    if (m_rawPoints.size() > 0 && m_rawPoints.back().x() == point.x() &&
        m_rawPoints.back().y() == point.y()) {
        return;
    }
    m_rawPoints.push_back(point);
    generatePoints();
}

//--------------------------------------------------------------------------------------------------

void SmoothStroke::endStroke() {
    generatePoints();
    // force enable the output all segments
    m_outputIndex = m_outputPoints.size() - 1;
}

//--------------------------------------------------------------------------------------------------

void SmoothStroke::clearPoints() {
    m_outputIndex = 0;
    m_readIndex = -1;
    m_outputPoints.clear();
    m_rawPoints.clear();
    m_resampledIndex = 0;
    m_resampledPoints.clear();
}

//--------------------------------------------------------------------------------------------------

void SmoothStroke::getSmoothPoints(std::vector<QPointF>& smoothPoints) {
    int n = m_outputPoints.size();
    for (int i = m_readIndex + 1; i <= m_outputIndex && i < n; ++i) {
        smoothPoints.push_back(m_outputPoints[i]);
    }
    m_readIndex = m_outputIndex;
}

//--------------------------------------------------------------------------------------------------

void SmoothStroke::generatePoints() {
    int n = (int)m_rawPoints.size();
    if (n == 0) {
        return;
    }

    // if m_smooth = 0, then skip whole smoothing process
    if (m_smooth == 0) {
        for (int i = m_outputIndex; i < (int)m_outputPoints.size(); ++i) {
            if (m_outputPoints[i] != m_rawPoints[i]) {
                break;
            }
            ++m_outputIndex;
        }
        m_outputPoints = m_rawPoints;
        return;
    }

    std::vector<QPointF> smoothedPoints = m_resampledPoints;
    // Add more stroke samples before applying the smoothing
    // This is because the raw inputs points are too few to support smooth result,
    // especially on stroke ends
    int resampleStartId = m_resampledIndex;
    for (int i = resampleStartId; i < n - 1; ++i) {
        const QPointF& p1 = m_rawPoints[i];
        const QPointF& p2 = m_rawPoints[i + 1];
        const QPointF& p0 = i - 1 >= 0 ? m_rawPoints[i - 1] : p1;
        const QPointF& p3 = i + 2 < n ? m_rawPoints[i + 2] : p2;

        std::vector<QPointF> tmpResampled;
        tmpResampled.push_back(p1);
        // define subsample amount according to distance between points
        int samples = std::min((int)tdistance(p1, p2), 8);
        if (samples >= 1)
            CatmullRomInterpolate(p0, p1, p2, p3, samples, tmpResampled);

        if (i + 2 < n) {
            m_resampledIndex = i + 1;
            std::copy(tmpResampled.begin(), tmpResampled.end(),
                std::back_inserter(m_resampledPoints));
        }
        std::copy(tmpResampled.begin(), tmpResampled.end(),
            std::back_inserter(smoothedPoints));
    }
    smoothedPoints.push_back(m_rawPoints.back());
    // Apply the 1D box filter
    // Multiple passes result in better quality and fix the stroke ends break
    // issue
    // level is passed to define range where the points are smoothed
    for (int level = 2; level >= 0; --level) {
        Smooth(smoothedPoints, m_smooth, m_readIndex, level);
    }
    // Compare the new smoothed stroke with old one
    // Enable the output for unchanged parts
    int outputNum = (int)m_outputPoints.size();
    for (int i = m_outputIndex; i < outputNum; ++i) {
        if (m_outputPoints[i] != smoothedPoints[i]) {
            break;
        }
        ++m_outputIndex;
    }
    m_outputPoints = smoothedPoints;
}
