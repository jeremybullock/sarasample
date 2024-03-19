#include "viewer.h"
#include "director.h"
#include "frame.h"
#include "motionpath.h"
#include "utilities.h"

#include <QGridLayout>
#include <QLabel>
#include <QFile>
#include <QApplication>
#include <QDockWidget>
#include <QFrame>

#include "stopmotion/stopmotion.h"
#include "stopmotion/stopmotioncontroller.h"

#include <QOpenGlWidget>
#include <QOpenGlFunctions>
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QWheelEvent>
#include <QTime>
#include <QFontMetrics>

bool areAlmostEqual(double a, double b, double err = 1e-8) {
    return fabs(a - b) < err;
}

class TCubic {

protected:
    QPointF m_p0, m_p1, m_p2, m_p3;

public:
    TCubic() : m_p0(), m_p1(), m_p2(), m_p3() {}

    //! p0,p1,p2,p3 are the four control points
    TCubic(const QPointF& p0, const QPointF& p1, const QPointF& p2,
        const QPointF& p3)
        : m_p0(p0), m_p1(p1), m_p2(p2), m_p3(p3) {}

    TCubic(const TCubic& src)
        : m_p0(src.m_p0), m_p1(src.m_p1), m_p2(src.m_p2), m_p3(src.m_p3) {}

    TCubic& operator=(const TCubic& src) {
        m_p0 = src.m_p0;
        m_p1 = src.m_p1;
        m_p2 = src.m_p2;
        m_p3 = src.m_p3;
        return *this;
    }

    QPointF getPoint(double t) const {
        double s = 1 - t;
        return m_p0 * s * s * s + 3 * t * s * (s * m_p1 + t * m_p2) +
            t * t * t * m_p3;
    }
};

double getCubicYfromX(TCubic c, double x, double& s0, double& s1) {
    double s = (s1 + s0) * 0.5;
    QPointF p = c.getPoint(s);
    if (areAlmostEqual(x, p.x(), 0.001)) return p.y();
    if (areAlmostEqual(s0, s1, 0.0001)) {
        return p.y();
    }

    if (x < p.x())
        return getCubicYfromX(c, x, s0, s);
    else
        return getCubicYfromX(c, x, s, s1);
}

//int getDevPixRatio() {
//    static int devPixRatio = this->screen()->devicePixelRatio();
//    return devPixRatio;
//}

Viewer::Viewer(QWidget* parent,
    Qt::WindowFlags f)
    : QOpenGLWidget(parent, f) {
    m_stopMotion = StopMotion::instance();
    m_director = Director::instance();
    QSurfaceFormat fmt;
    fmt.setSamples(8);
    this->setFormat(fmt);
    connect(m_stopMotion, &StopMotion::newLiveViewImageReady,
        this, &Viewer::getLiveViewImage);
    connect(m_stopMotion, &StopMotion::liveViewStopped,
        this, &Viewer::liveViewStopped);
    connect(Director::instance(), &Director::newPathSelected,
        this, &Viewer::onPathSelectedSignal);
}

Viewer::~Viewer() {};
    

void Viewer::liveViewStopped() {
    update();
}

void Viewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    bool showLiveView = false;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    bool flip = m_director->getFlipHoriz();
    // deal with if playing first

    painter.fillRect(0, 0, this->width(), this->height(), m_backgroundColor);
    QRectF target;
    double ratio = 0.0;
    if (m_showTestShot && m_testImage.height() > 0) {
        if (!m_scaleSet) getScale();

        int imgHeight = m_testImage.height();
        int imgWidth = m_testImage.width();
        ratio = (double)imgWidth / (double)imgHeight;

        int screenImgWidth = m_imgWidth * m_scale * this->screen()->devicePixelRatio();
        int screenImgHeight = (double)screenImgWidth / ratio;

        double new_x = (this->width()) / 2.0 - (screenImgWidth / 2.0);
        double new_y = (this->height()) / 2.0 - (screenImgHeight / 2.0);
        target = QRectF(new_x + m_viewOffset.x(), new_y + m_viewOffset.y(), screenImgWidth, screenImgHeight);
        if (flip) {
            painter.drawImage(target, m_testImage.mirrored(true, false), m_testImage.rect());
        }
        else {
            painter.drawImage(target, m_testImage, m_testImage.rect());
        }
        if (m_scale > m_maxScale) {
            painter.setPen(Qt::red);
            painter.drawRect(1, 0, this->width() - 1, this->height() - 1);
        }
    }

    else if (m_director->getPlaying() && m_currentImage.height() > 0) {
        int imgHeight = m_currentImage.height();
        int imgWidth = m_currentImage.width();
        ratio = (double)imgWidth / (double)imgHeight;

        int screenImgWidth = m_imgWidth * m_scale * this->screen()->devicePixelRatio();
        int screenImgHeight = (double)screenImgWidth / ratio;

        double new_x = (this->width()) / 2.0 - (screenImgWidth / 2.0);
        double new_y = (this->height()) / 2.0 - (screenImgHeight / 2.0);
        target = QRectF(new_x + m_viewOffset.x(), new_y + m_viewOffset.y(), screenImgWidth, screenImgHeight);
        if (flip) {
            painter.drawImage(target, m_currentImage.mirrored(true, false), m_currentImage.rect());
        }
        else {
            painter.drawImage(target, m_currentImage, m_currentImage.rect());
        }
    }
    //else if (!m_showLiveView) {
    else if (m_liveViewShownOnce && m_director->getCameraFrame() == m_director->getCurrentFrame() && m_stopMotion->m_liveViewStatus == StopMotion::LiveViewClosed) {
        QFont font("Segoe UI", 48);
        QFontMetrics fm(font);
        int pixelsWide = fm.horizontalAdvance("Live View Stopped");
        int pixelsHigh = fm.height();
        //int pixelsWide = fm.maxWidth();
        int halfWidth = pixelsWide / 2;
        int startX = this->width() / 2 - halfWidth;
        int startY = this->height() / 2 - pixelsHigh;
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QPoint(startX, startY), "Live View Stopped");
        
        pixelsWide = fm.horizontalAdvance("Press 5 to Restart Live View.");
        pixelsHigh = fm.height();
        
        halfWidth = pixelsWide / 2;
        startX = this->width() / 2 - halfWidth;
        startY = this->height() / 2 + 15;
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(QPoint(startX, startY), "Press 5 to Restart Live View.");

    }
    // next deal with camera and live view
    else if ((m_director->getCameraFrame() == m_director->getCurrentFrame()) && m_liveViewImage.height() > 0) {
        if (!m_scaleSet) getScale();
        showLiveView = true;
        m_liveViewShownOnce = true;
        int imgHeight = m_liveViewImage.height();
        int imgWidth = m_liveViewImage.width();
        ratio = (double)imgWidth / (double)imgHeight;

        int screenImgWidth = m_imgWidth * m_scale * this->screen()->devicePixelRatio();
        int screenImgHeight = (double)screenImgWidth / ratio;

        double new_x = (this->width()) / 2.0 - (screenImgWidth / 2.0);
        double new_y = (this->height()) / 2.0 - (screenImgHeight / 2.0);
        target = QRectF(new_x + m_viewOffset.x(), new_y + m_viewOffset.y(), screenImgWidth, screenImgHeight);
        painter.setOpacity(1.0);
        if (flip) {
            QImage flipped(m_liveViewImage.width(), m_liveViewImage.height(), m_liveViewImage.format());
            QPainter otherPainter(&flipped);
            otherPainter.begin(&flipped);
            otherPainter.translate(flipped.width(), 0);
            otherPainter.scale(-1, 1);
            if (m_stopMotion->m_canon->m_zooming) {
            }
            else if (m_director->getCameraFrame() > 0 && m_lineUpImage.height() > 0) {
                otherPainter.drawImage(m_liveViewImage.rect(), m_lineUpImage, m_lineUpImage.rect());
                otherPainter.setOpacity(m_director->getOpacity());
            }

            otherPainter.drawImage(m_liveViewImage.rect(), m_liveViewImage, m_liveViewImage.rect(), Qt::ColorOnly);
            otherPainter.end();
            painter.drawImage(target, flipped, flipped.rect(), Qt::ColorOnly);
        }
        else {
            if (m_stopMotion->m_canon->m_zooming) {
            }
            else if (m_director->getCameraFrame() > 0 && m_lineUpImage.height() > 0) {
                painter.drawImage(target, m_lineUpImage, m_lineUpImage.rect());
                painter.setOpacity(m_director->getOpacity());
            }
            painter.drawImage(target, m_liveViewImage, m_liveViewImage.rect());
        }

        painter.setOpacity(1.0);
        if (m_scale > m_maxScale) {
            painter.setPen(Qt::red);
            painter.drawRect(1, 0, this->width()-1, this->height()-1);
        }
    }

    else if (m_director->getCurrentFrame() != m_director->getCameraFrame() && m_currentImage.height() > 0) {
        if (!m_scaleSet) getScale();

        int imgHeight = m_currentImage.height();
        int imgWidth = m_currentImage.width();
        ratio = (double)imgWidth / (double)imgHeight;

        int screenImgWidth = m_imgWidth * m_scale * this->screen()->devicePixelRatio();
        int screenImgHeight = (double)screenImgWidth / ratio;

        double new_x = (this->width()) / 2.0 - (screenImgWidth / 2.0);
        double new_y = (this->height()) / 2.0 - (screenImgHeight / 2.0);
        target = QRectF(new_x + m_viewOffset.x(), new_y + m_viewOffset.y(), screenImgWidth, screenImgHeight);
        if (flip) {
            painter.drawImage(target, m_currentImage.mirrored(true, false), m_currentImage.rect());
        }
        else {
            painter.drawImage(target, m_currentImage, m_currentImage.rect());
        }
        if (m_scale > m_maxScale) {
            painter.setPen(Qt::red);
            painter.drawRect(1, 0, this->width() - 1, this->height() - 1);
        }
    }

    painter.setRenderHint(QPainter::Antialiasing);
    //painter.setRenderHint(QPainter::HighQualityAntialiasing);

    double overlayNewTop = 0.0;
    double overlayNewHeight = 0.0;

    // draw overlays
    if (m_director->getShowOverlays() && target.width() > 0) {

        double width = target.width();
        overlayNewHeight = width / m_overlayRatio;

        double difference = target.height() - overlayNewHeight;
        int yOffset = difference / 2.0;
        overlayNewTop = target.top() + yOffset;

        double top = target.top();
        double bottom = target.bottom();
        double left = target.left();
        double right = target.right();

        QColor color = m_backgroundColor;
        color.setAlpha(m_overlayOpacity);

        QRectF topRect = QRectF(left, top, width, yOffset);
        QRectF bottomRect = QRectF(left, bottom - yOffset, width, yOffset);

        painter.fillRect(topRect, color);
        painter.fillRect(bottomRect, color);
    }

    // draw rule of thirds
    if (!m_director->getPlaying() && m_director->getShowRuleOfThirds() && target.width() > 0) {
        double top;
        double bottom;
        int yOffset;
        int xOffset = target.width() / 3.0;

        if (overlayNewTop != 0.0) {
            top = overlayNewTop;
            bottom = top + overlayNewHeight;
            yOffset = overlayNewHeight / 3.0;
        }
        else {
            top = target.top();
            bottom = target.bottom();
            yOffset = target.height() / 3.0;
        }
        double left = target.left();
        double right = target.right();

        QColor white = Qt::white;
        white.setAlpha(100);
        painter.setPen(white);
        
        painter.drawLine(left, top + yOffset, right, top + yOffset);
        painter.drawLine(left, top + yOffset * 2.0, right, top + yOffset * 2.0);
        painter.drawLine(left + xOffset, top, left + xOffset, bottom);
        painter.drawLine(left + xOffset * 2.0, top, left + xOffset * 2.0, bottom);

        QColor black = Qt::black;
        black.setAlpha(100);
        painter.setPen(black);
        painter.drawLine(left, top + yOffset + 1, right, top + yOffset + 1);
        painter.drawLine(left, top + yOffset * 2.0 + 1, right, top + yOffset * 2.0 + 1);
        painter.drawLine(left + xOffset + 1, top, left + xOffset + 1, bottom);
        painter.drawLine(left + xOffset * 2.0 + 1, top, left + xOffset * 2.0 + 1, bottom);
    }

    // draw the test shot text and the close button for the test
    if (m_showTestShot && m_testImage.height() > 0) {
        const QRect rectangle = QRect(10, 10, 5000, 500);
        QRect boundingRect;
        if (m_backgroundColor.red() > 128) {
            painter.setPen(Qt::black);
        }
        else {
            painter.setPen(Qt::white);
        }
        painter.drawText(rectangle, Qt::TextWordWrap, m_testInfo, &boundingRect);
        QImage redX;
        bool loaded = redX.load(":/Resources/red_x.svg");
        painter.drawImage(QRect(this->width() - 40, 10, 30, 30), redX, redX.rect());
    }
    // draw the red dot for the camera frame
    else if ((m_director->getCameraFrame() == m_director->getCurrentFrame()) && m_liveViewImage.height() > 0 && showLiveView) {
        painter.setPen(Qt::red);
        painter.setBrush(Qt::red);
        painter.drawEllipse(QRect(12.0, 12.0, 12.0, 12.0));
    }

    painter.setPen(Qt::green);
    QBrush emptyBrush;
    painter.setBrush(emptyBrush);


    if (m_scaleSet && !m_showTestShot && !m_director->getPlaying()) {

        // draw motion paths being created
        if (m_dragging && m_director->getActiveTool() == Director::LINE && m_lastClickPos != QPointF(-1.0, -1.0)) {
            painter.drawLine(translatePointToScreen(m_firstClickPos)  + m_viewOffset, translatePointToScreen(m_lastClickPos) + m_viewOffset);
        }
        else if (m_dragging && m_director->getActiveTool() == Director::RECTANGLE && m_lastClickPos != QPointF(-1.0, -1.0)) {
            QRectF rect = getScaledShapeRect();
            painter.drawRect(QRectF(rect.topLeft() + m_viewOffset, rect.size()));
        }
        else if (m_dragging && m_director->getActiveTool() == Director::CIRCLE && m_lastClickPos != QPointF(-1.0, -1.0)) {
            QRectF rect = getScaledShapeRect();
            painter.drawEllipse(QRectF(rect.topLeft() + m_viewOffset, rect.size()));
        }
        std::vector<QPointF> strokePoints = m_drawing.getPoints();
        QPainterPath strokePath;
        if (strokePoints.size() > 0) {
            strokePath.moveTo(translatePointToScreen(strokePoints.at(0)) + m_viewOffset);
            for (auto point : strokePoints) {
                strokePath.lineTo(translatePointToScreen(point) + m_viewOffset);
            }
            QPen strokePen(Qt::blue);
            painter.strokePath(strokePath, strokePen);
        }


        // draw existing motion paths
        std::vector<MotionPath> paths = m_director->getMotionPaths();
        for (int i = 0; i < paths.size(); i++) {
            MotionPath path = paths.at(i);
            QPainterPath painterPath = path.getPainterPath();
            QPen pathPen(path.getColor());
            pathPen.setWidth(path.getWidth());
            painter.translate(m_viewOffset);
            painter.strokePath(painterPath, pathPen);
            painter.resetTransform();
            QPointF circleAt = QPointF(0.0, 0.0);
            if (true || path.getType() == MotionPath::PATH) {
                // draw step points
                std::vector<std::pair<QPointF, double>> stepPoints = path.getStepPoints();
                for (int j = 0; j < stepPoints.size(); j++) {
                    std::pair<QPointF, double> stepPoint = stepPoints.at(j);
                    painter.setPen(pathPen);
                    painter.translate(stepPoint.first + m_viewOffset);
                    painter.rotate(stepPoint.second);
                    painter.drawLine(QPointF(0.0, 15.0 * m_scale), QPointF(0.0, -15.0 * m_scale));
                    int currentStep = path.getCurrentStep();
                    if (m_director->getPathIsPlaying() && m_director->getSelectedPath() == i && path.getCurrentStep() == j) {
                        circleAt = stepPoint.first;
                    }
                    painter.resetTransform();
                }
                if (m_director->getPathIsPlaying() && m_director->getSelectedPath() == i && circleAt != QPointF(0.0, 0.0)) {
                    painter.translate(circleAt + m_viewOffset);
                    QPainterPath circle;
                    circle.addEllipse(QPointF(0, 0), 20.0 * m_scale, 20.0 * m_scale);
                    QBrush circleBrush(Qt::red);
                    painter.fillPath(circle, circleBrush);
                    painter.resetTransform();
                }
            }

            // draw the rect around the selected path
            if (m_highlighPath == i) {
                QPen highlightPen;
                highlightPen.setStyle(Qt::DashLine);
                QVector<qreal> dashes;
                qreal space = 4;
                dashes << 9 << space;
                highlightPen.setDashPattern(dashes);
                highlightPen.setColor(QColor(150, 150, 150, 150));
                painter.setPen(highlightPen);
                QRectF rect = painterPath.boundingRect();
                rect = QRectF(rect.topLeft() + m_viewOffset, rect.size());
                painter.drawRect(rect);
                if (m_director->getActiveTool() == Director::MOVE) {
                    double x = rect.left();
                    double y = rect.top();
                    double width = rect.width();
                    double height = rect.height();
                    double halfWidth = width / 2.0;
                    double halfHeight = height / 2.0;

                    m_topLeft = QRect(x - 5, y - 5, 10, 10);
                    m_topCenter = QRect(x + halfWidth - 5, y - 5, 10, 10);
                    m_topRight = QRect(x + width - 5, y - 5, 10, 10);

                    m_left = QRect(x - 5, y + halfHeight - 5, 10, 10);
                    m_right = QRect(x + width - 5, y + halfHeight - 5, 10, 10);


                    m_bottomLeft = QRect(x - 5, y + height - 5, 10, 10);
                    m_bottomCenter = QRect(x + halfWidth - 5, y + height - 5, 10, 10);
                    m_bottomRight = QRect(x + width - 5, y + height - 5, 10, 10);

                    highlightPen.setStyle(Qt::SolidLine);
                    painter.setPen(highlightPen);
                    painter.drawRect(m_topLeft);
                    painter.drawRect(m_topCenter);
                    painter.drawRect(m_topRight);
                    painter.drawRect(m_left);
                    painter.drawRect(m_right);
                    painter.drawRect(m_bottomLeft);
                    painter.drawRect(m_bottomCenter);
                    painter.drawRect(m_bottomRight);
                }
            }
        }
    }

    // draw the rect for picking canon zoom
    if (!m_showTestShot && m_stopMotion->m_canon->m_pickLiveViewZoom && (m_director->getCurrentFrame() == m_director->getCameraFrame())) {
        QRectF pickRect = m_stopMotion->m_canon->m_zoomRect;
        QPainterPath zoomPath;
        double zoomScale = 1920.0 / m_stopMotion->m_canon->m_fullImageDimensions.x();
        double offsetX = pickRect.width() / 2.0 * zoomScale;
        double offsetY = pickRect.height() / 2.0 * zoomScale;
        QPointF zoomPoint = m_stopMotion->m_canon->m_liveViewZoomPickPoint;

        zoomPath.moveTo(translatePointToScreen(QPointF(zoomPoint.x() - offsetX, zoomPoint.y() - offsetY), true) + m_viewOffset);
        zoomPath.lineTo(translatePointToScreen(QPointF(zoomPoint.x() + offsetX, zoomPoint.y() - offsetY), true) + m_viewOffset);
        zoomPath.lineTo(translatePointToScreen(QPointF(zoomPoint.x() + offsetX, zoomPoint.y() + offsetY), true) + m_viewOffset);
        zoomPath.lineTo(translatePointToScreen(QPointF(zoomPoint.x() - offsetX, zoomPoint.y() + offsetY), true) + m_viewOffset);
        zoomPath.closeSubpath();
        QPen zoomPen(Qt::white);
        painter.strokePath(zoomPath, zoomPen);
    }
}

void Viewer::getLiveViewImage() {
    if (m_stopMotion->m_hasLiveViewImage) {
        m_liveViewImage = m_stopMotion->m_liveViewImage;
        this->repaint();
    }
}

void Viewer::wheelEvent(QWheelEvent* event) {
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;
    double oldScale = m_scale;
    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        if (m_scale + ((double)numSteps.y() * 0.1) > 0.0) {
            m_scale += numSteps.y() * 0.1;
        }
    }
    double percentChange = m_scale / oldScale;
    m_viewOffset *= percentChange;
    setPaths();
    event->accept();
    this->repaint();
}


// used to handle mouse events and set the event relative to a 1080p space
void Viewer::translateClickPos() {
    int imgHeight = m_imgHeight * m_scale;
    int imgWidth = m_imgWidth * m_scale;
    double ratio = (double)imgWidth / (double)imgHeight;
    imgHeight = (double)imgWidth / ratio;
    double centerX = this->width() / 2;
    double centerY = this->height() / 2;
    double newX = centerX - (imgWidth / 2);
    double newY = centerY - (imgHeight / 2);
    m_clickPos = m_realClickPos;
    m_clickPos.setX(m_clickPos.x() - newX);
    m_clickPos.setY(m_clickPos.y() - newY);
    m_clickPos /= m_scale;
    int x = 3;
}


// used to draw paths and live view zoom rectangle
QPointF Viewer::translatePointToScreen(QPointF point, bool useImage) {
    int imgHeight, imgWidth, lvImgHeight, lvImgWidth;
    double ratio = 0.0;
    if (useImage && m_liveViewImage.width() > 0) {
        lvImgHeight = m_liveViewImage.height() * m_scale;
        lvImgWidth = m_liveViewImage.width() * m_scale;
        ratio = (double)lvImgWidth / (double)lvImgHeight;
    }
    imgHeight = m_imgHeight * m_scale;
    imgWidth = m_imgWidth * m_scale;

    point = QPointF(point.x() * m_scale, point.y() * m_scale);
    if (ratio == 0.0) ratio = (double)imgWidth / (double)imgHeight;
    imgHeight = (double)imgWidth / ratio;
    double new_x = (this->width()) / 2 - (imgWidth / 2);
    double new_y = (this->height()) / 2 - (imgHeight / 2);
    return QPointF(new_x + point.x(), new_y + point.y());
}

// translates click points relative to image position
void Viewer::translateClickPointToImage() {
    int imgHeight = m_liveViewImage.height() * m_scale;
    int imgWidth = m_liveViewImage.width() * m_scale;
    double ratio = (double)imgWidth / (double)imgHeight;
    imgHeight = (double)imgWidth / ratio;

    int screenImgHeight = m_imgHeight * m_scale;
    int screenImgWidth = m_imgWidth * m_scale;
    screenImgHeight = (double)screenImgWidth / ratio;

    double centerX = this->width() / 2;
    double centerY = this->height() / 2;
    double newX = centerX - (screenImgWidth / 2);
    double newY = centerY - (screenImgHeight / 2);
    
    m_clickPos = m_realClickPos;

    m_clickPos.setX(m_clickPos.x() - newX);
    m_clickPos.setY(m_clickPos.y() - newY);
    m_clickPos /= m_scale;
    int x = 3;
}

void Viewer::contextMenuEvent(QContextMenuEvent* event) {
    m_lastPos = event->pos();
    m_realClickPos = event->pos();
    translateClickPos();
    //m_director->handleRightClick(m_clickPos, event->globalPos());
    this->update();
}

void Viewer::mousePressEvent(QMouseEvent* event) {
    if (event->buttons() == Qt::LeftButton) {
        m_showLiveViewTemp = m_director->getShowLiveView();
        m_director->setShowLiveView(false);
        m_setLiveView = true;
        m_lastPos = event->pos();
        m_realClickPos = event->localPos() - m_viewOffset;
        int width = this->width();

        QRectF rect(width - 40, 0, 40, 40);
        if (m_showTestShot) {
            if (rect.contains(event->localPos())) {
                m_showTestShot = false;
            }
        }
        else if (m_stopMotion->m_canon->m_pickLiveViewZoom) {
            translateClickPointToImage();
            m_stopMotion->m_canon->makeZoomPoint(m_clickPos);
        }
        else if (m_director->getActiveTool() == Director::PATH) {
            translateClickPos();
            m_dragging = true;
            m_drawing.leftButtonDown(m_clickPos, *event);
        }
        else if (m_director->getActiveTool() == Director::MOVE) {
            // first check if resizing a current path
            m_changingTop = m_topLeft.contains(event->localPos()) || m_topCenter.contains(event->localPos()) || m_topRight.contains(event->localPos());
            m_changingBottom = m_bottomLeft.contains(event->localPos()) || m_bottomCenter.contains(event->localPos()) || m_bottomRight.contains(event->localPos());
            m_changingLeft = m_topLeft.contains(event->localPos()) || m_left.contains(event->localPos()) || m_bottomLeft.contains(event->localPos());
            m_changingRight = m_topRight.contains(event->localPos()) || m_right.contains(event->localPos()) || m_bottomRight.contains(event->localPos());

            translateClickPos();

            if (m_highlighPath > -1 && (m_changingTop || m_changingBottom || m_changingLeft || m_changingRight)) {
                m_dragging = true;
            }

            // not resizing
            else {

                int index = Utilities::instance()->checkSelection(m_clickPos);
                if (index != m_highlighPath) {
                    m_director->setSelectedPath(index);
                    m_highlighPath = index;
                }
                if (index > -1) {
                    m_dragging = true;
                }
                else {
                    m_director->setShowLiveView(m_showLiveViewTemp);
                    m_setLiveView = false;
                }
            }
        }
        else if (m_director->getActiveTool() == Director::LINE ||
            m_director->getActiveTool() == Director::CIRCLE ||
            m_director->getActiveTool() == Director::RECTANGLE) {
            translateClickPos();
            m_firstClickPos = m_clickPos;
            m_dragging = true;
        }
        //m_director->handleMouseClick(m_clickPos);
        this->update();
        m_mouseDown = true;
    }
}

void Viewer::mouseMoveEvent(QMouseEvent* event) {
    m_shift = QGuiApplication::keyboardModifiers() == Qt::ShiftModifier;
    if (m_stopMotion->m_canon->m_pickLiveViewZoom) return;
    if (event->buttons() == Qt::LeftButton) {
        QPointF distance = event->pos() - m_lastPos;
        if (event->pos() != m_lastPos) {
            m_lastPos = event->pos();
            m_realClickPos = event->localPos() - m_viewOffset;

            if (m_director->getActiveTool() == Director::PATH && m_dragging) {
                //m_director->handleMouseMove(m_clickPos, shift);
                translateClickPos();
                m_drawing.leftButtonDrag(m_clickPos, *event);
                this->repaint();
            }
            else if (m_director->getActiveTool() == Director::MOVE && m_dragging) {
                QPointF lastClickPos = m_clickPos;
                translateClickPos();
                if (m_highlighPath > -1 && (m_changingTop || m_changingBottom || m_changingLeft || m_changingRight)) {
                    m_director->scaleMotionPath(m_highlighPath, m_changingTop, m_changingBottom, m_changingLeft, m_changingRight, m_clickPos, m_shift, true, false);
                    setPaths();
                }
                else if (m_highlighPath > -1 && m_highlighPath < m_director->getMotionPathCount()) {
                    m_director->moveMotionPath(m_highlighPath, m_clickPos - lastClickPos, true, false);
                    setPaths();
                }
                this->repaint();
            }
            else if (m_dragging && (m_director->getActiveTool() == Director::LINE ||
                m_director->getActiveTool() == Director::CIRCLE ||
                m_director->getActiveTool() == Director::RECTANGLE)) {
                translateClickPos();
                m_lastClickPos = m_clickPos;
                this->repaint();
            }
            else if (m_director->getActiveTool() == Director::MOVE) {
                m_viewOffset += distance;
                this->repaint();
            }
        }   
    }
}

void Viewer::mouseReleaseEvent(QMouseEvent* event) {
    if (m_setLiveView) {
        m_director->setShowLiveView(m_showLiveViewTemp);
    }
    if (m_stopMotion->m_canon->m_pickLiveViewZoom) return;
    if (m_director->getActiveTool() == Director::PATH && m_dragging && event->button() == Qt::LeftButton) {
        m_realClickPos = event->localPos() - m_viewOffset;
        translateClickPos();
        m_drawing.leftButtonUp(m_clickPos, *event);
        if (m_drawing.getBezierPoints().size() > 4) {
            MotionPath path;
            path.setPoints(m_drawing.getBezierPoints());
            path.setRawPoints(m_drawing.getPoints());
            m_director->addMotionPath(path);
        }
        m_drawing.clearPoints();
        this->update();
    }
    else if (m_dragging && m_director->getActiveTool() == Director::LINE && m_lastClickPos != QPointF(-1.0, -1.0)) {
        MotionPath path;
        path.setType(MotionPath::LINE);
        QList<QPointF> points;
        points.push_back(m_firstClickPos);
        points.push_back(m_lastClickPos);
        path.setPoints(points);
        m_director->addMotionPath(path);
    }
    else if (m_dragging && m_director->getActiveTool() == Director::RECTANGLE && m_lastClickPos != QPointF(-1.0, -1.0)) {
        QRectF rect = getShapeRect();
        MotionPath path;
        path.setType(MotionPath::RECTANGLE);
        QList<QPointF> points;
        points.push_back(rect.topLeft());
        points.push_back(rect.bottomRight());
        path.setPoints(points);
        m_director->addMotionPath(path);
    }
    else if (m_dragging && m_director->getActiveTool() == Director::CIRCLE && m_lastClickPos != QPointF(-1.0, -1.0)) {
        QRectF rect = getShapeRect();
        MotionPath path;
        path.setType(MotionPath::CIRCLE);
        QList<QPointF> points;
        points.push_back(rect.topLeft());
        points.push_back(rect.bottomRight());
        path.setPoints(points);
        m_director->addMotionPath(path);
    }
    else if (m_director->getActiveTool() == Director::MOVE && m_dragging) {
        QPointF lastClickPos = m_clickPos;
        m_realClickPos = event->localPos() - m_viewOffset;
        translateClickPos();
        if (m_highlighPath > -1 && (m_changingTop || m_changingBottom || m_changingLeft || m_changingRight)) {
            m_director->scaleMotionPath(m_highlighPath, m_changingTop, m_changingBottom, m_changingLeft, m_changingRight, m_clickPos, m_shift, false, true);
            setPaths();
        }
        if (m_highlighPath < m_director->getMotionPathCount()) {
            m_director->moveMotionPath(m_highlighPath, m_clickPos - lastClickPos, false, true);
            setPaths();
        }
    }
    m_mouseDown = false;
    m_dragging = false;
    m_lastClickPos = QPointF(-1.0, -1.0);
    m_setLiveView = false;
    m_changingBottom = false;
    m_changingTop = false;
    m_changingLeft = false;
    m_changingRight = false;
    m_shift = false;
}

bool Viewer::event(QEvent* e) {
    // discard too frequent move events
    static QTime clock;
    if (e->type() == QEvent::MouseButtonPress)
        clock = QTime::currentTime().addMSecs(40);
    else if (e->type() == QEvent::MouseMove) {
        if (clock.isValid() && QTime::currentTime() < clock) {
            e->accept();
            return true;
        }

        clock = QTime::currentTime().addMSecs(40);
    }

    return QOpenGLWidget::event(e);
}

QRectF Viewer::getScaledShapeRect() {
    QPointF first = translatePointToScreen(m_firstClickPos);
    QPointF last = translatePointToScreen(m_lastClickPos);

    double xDistance = last.x() - first.x();
    double yDistance = last.y() - first.y();

    double width = std::abs(first.x() - last.x());
    double height = std::abs(first.y() - last.y());

    // get the top left corner
    double x = std::min(first.x(), last.x());
    double y = std::min(first.y(), last.y());

    if (m_shift) {
        //yDistance = xDistance;
        height = width;

        if (yDistance < 0) {
            y = first.y() - width;
        }
    }

    return QRectF(x, y, width, height);
}

QRectF Viewer::getShapeRect() {
    QPointF first = m_firstClickPos;
    QPointF last = m_lastClickPos;

    double xDistance = last.x() - first.x();
    double yDistance = last.y() - first.y();
    
    double width = std::abs(first.x() - last.x());
    double height = std::abs(first.y() - last.y());

    // get the top left corner
    double x = std::min(first.x(), last.x());
    double y = std::min(first.y(), last.y());

    if (m_shift) {
        //yDistance = xDistance;
        height = width;

        if (yDistance < 0) {
            y = first.y() - width;
        }
    }

    return QRectF(x, y, width, height);
}

void Viewer::onPathSelectedSignal(int path) {
    m_highlighPath = path;
}

void Viewer::resizeEvent(QResizeEvent* event) {
    m_scaleSet = false;
    QOpenGLWidget::resizeEvent(event);
    setPaths();
}

void Viewer::getScale() {
    double screenRatio = this->screen()->devicePixelRatio();
    int height = this->height() / screenRatio;
    int width = this->width() / screenRatio;
    int imgHeight = m_liveViewImage.height();
    int imgWidth = m_liveViewImage.width();
    if (imgHeight == 0 && m_currentImage.height() > 0) {
        imgHeight = m_currentImage.height();
        imgWidth = m_currentImage.width();
    }
    double ratio = (double)imgWidth / (double)imgHeight;
    imgHeight = (double)imgWidth / ratio;
    imgWidth = 1920;
    imgHeight = (double)imgWidth / ratio;
    bool scaleDown = false;
    if (imgHeight > height - 20) scaleDown = true;
    if (imgWidth > width - 20) scaleDown = true;
    if (scaleDown) {
        int xDist = width - 20 - imgWidth;
        int yDist = height - 20 - imgHeight;
        if (xDist < yDist) {
            m_scale = (double)(width - 20) / (double)imgWidth;
            if ((double)(imgHeight) * m_scale > height - 20) {
                m_scale = (double)(height - 20) / (double)imgHeight;
            }
        }
        else {
            m_scale = (double)(height - 20) / (double)imgHeight;
            if ((double)(imgWidth) * m_scale > width - 20) {
                m_scale = (double)(width - 20) / (double)imgWidth;
            }
        }
    }

    bool scaleUp = false;
    if (imgHeight < height - 20) scaleUp = true;
    if (imgWidth < width - 20) scaleUp = true;
    if (scaleUp) {
        int xDist = width - 20 - imgWidth;
        int yDist = height - 20 - imgHeight;
        if (xDist > yDist) {
            m_scale = (double)(width - 20) / (double)imgWidth;
            if ((double)(imgHeight)*m_scale > height - 20) {
                m_scale = (double)(height - 20) / (double)imgHeight;
            }
        }
        else {
            m_scale = (double)(height - 20) / (double)imgHeight;
            if ((double)(imgWidth)*m_scale > width - 20) {
                m_scale = (double)(width - 20) / (double)imgWidth;
            }
        }
    }

    m_maxScale = m_scale;
    m_scaleSet = true;
    setPaths();
}

void Viewer::fitViewer(bool ignore) {
    m_scale = m_maxScale;
    m_viewOffset = QPointF(0.0, 0.0);
    update();
}


void Viewer::setPaths() {
    std::vector<MotionPath> paths = m_director->getMotionPaths();
    for (int i = 0; i < paths.size(); i++) {
        MotionPath path = paths.at(i);
        QPainterPath painterPath;
        QPainterPath originalPath;
        QPainterPath pointPath;
        QPainterPath rawPath;
        QList<QPointF> interpolationStroke = path.getIterpolationPoints();
        QList<QPointF> points = path.getPoints();
        if (path.getType() == MotionPath::PATH) {

            std::vector<QPointF> rawPathPoints = path.getRawPoints();
            if (rawPathPoints.size() > 0) {
                rawPath.moveTo(translatePointToScreen(rawPathPoints.at(0)));
                for (int j = 1; j < rawPathPoints.size(); j++) {
                    rawPath.lineTo(translatePointToScreen(rawPathPoints.at(j)));
                }
            }

            if (points.size() > 0) {
                pointPath.moveTo(translatePointToScreen(points.at(1)));
                originalPath.moveTo(points.at(1));
            }

            for (int k = 4; k < points.size(); ) {
                pointPath.cubicTo(translatePointToScreen(points[k - 2]), translatePointToScreen(points[k - 1]), translatePointToScreen(points[k]));
                originalPath.cubicTo(points[k - 2], points[k - 1], points[k]);
                k += 3;
            }
            m_director->setMotionPathPainterPath(i, pointPath);
            m_director->setMotionPathOriginalPainterPath(i, originalPath);
        }
        else if (path.getType() == MotionPath::RECTANGLE) {
            
            painterPath.addRect(QRectF(translatePointToScreen(points.at(0)), translatePointToScreen(points.at(1))));
            originalPath.addRect(QRectF(points.at(0), points.at(1)));
            m_director->setMotionPathPainterPath(i, painterPath);
            m_director->setMotionPathOriginalPainterPath(i, originalPath);
        }
        else if (path.getType() == MotionPath::CIRCLE) {
            painterPath;
            originalPath;
            painterPath.addEllipse(QRectF(translatePointToScreen(points.at(0)), translatePointToScreen(points.at(1))));
            originalPath.addEllipse(QRectF(points.at(0), points.at(1)));
            m_director->setMotionPathPainterPath(i, painterPath);
            m_director->setMotionPathOriginalPainterPath(i, originalPath);
        }
        else if (path.getType() == MotionPath::LINE) {
            painterPath;
            originalPath;
            painterPath.moveTo(translatePointToScreen(points.at(0)));
            painterPath.lineTo(translatePointToScreen(points.at(1)));
            originalPath.moveTo(points.at(0));
            originalPath.lineTo(points.at(1));
            m_director->setMotionPathPainterPath(i, painterPath);
            m_director->setMotionPathOriginalPainterPath(i, originalPath);
        }
        // now draw the step points
        int steps = path.getStepCount();
        double stepIncrement = 1.0 / (double)(steps > 1 ? steps - 1 : 1);
        std::vector<std::pair<QPointF, double>> stepPoints;
        if (points.size() > 0 && path.getStepCount() > 0) {
            // lets figure out the y value for our x position

            int cp = 3;
            QPointF startPoint = interpolationStroke.at(cp);
            QPointF control1 = interpolationStroke.at(cp + 1);
            QPointF control2 = interpolationStroke.at(cp + 2);
            QPointF endPoint = interpolationStroke.at(cp + 3);
            TCubic cubic(startPoint, control1, control2, endPoint);

            //double length = stroke->getLength(0.0, 1.0);
            double step = 1.0 / (double)(steps > 1 ? steps - 1 : 1);

            double currentPosition = 0.0;
            double s0 = 0.0;
            double s1 = 1.0;

            QPointF prePoint, point, postPoint;
            for (int l = 0; l <= steps; l++) {
                double y = -1;
                if (l == 0)
                    y = 0;
                else if (l == steps)
                    y = 1000.0;
                else {
                    currentPosition = (double)l * stepIncrement;
                    if (currentPosition > 1.0) currentPosition = 1.0;
                    double tempX = std::round(currentPosition * 1000.0);
                    if (tempX > endPoint.x()) {
                        cp += 3;
                        startPoint = interpolationStroke.at(cp);
                        control1 = interpolationStroke.at(cp + 1);
                        control2 = interpolationStroke.at(cp + 2);
                        endPoint = interpolationStroke.at(cp + 3);
                        cubic = TCubic(startPoint, control1, control2, endPoint);
                    }
                    y = getCubicYfromX(cubic, tempX, s0, s1);
                }

                double newPercent = y / 1000.0;
                QPointF beforeHalfWay;
                QPointF halfWay;
                QPointF pastHalfWay;
                if (path.getType() == MotionPath::PATH) {
                    beforeHalfWay = rawPath.pointAtPercent(std::max(0.0, newPercent - 0.05));
                    halfWay = rawPath.pointAtPercent(newPercent);
                    pastHalfWay = rawPath.pointAtPercent(std::min(1.0, newPercent + 0.05));
                }
                else {
                    beforeHalfWay = painterPath.pointAtPercent(std::max(0.0, newPercent - 0.05));
                    halfWay = painterPath.pointAtPercent(newPercent);
                    pastHalfWay = painterPath.pointAtPercent(std::min(1.0, newPercent + 0.05));
                }

                double newAngle = atan2(pastHalfWay.y() - beforeHalfWay.y(), pastHalfWay.x() - beforeHalfWay.x());

                stepPoints.push_back(std::pair<QPointF, double>(halfWay, newAngle * 57.2957795));
            }
        }
        m_director->setMotionPathStepPoints(i, stepPoints);
    }
}

