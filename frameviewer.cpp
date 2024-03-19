#include "frameviewer.h"
#include "frame.h"
#include "stopmotion/stopmotion.h"
#include "director.h"
#include "clipboard.h"
#include "commands.h"
#include "mainwindow.h"

#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QMenu>

FrameViewer::FrameViewer(QScrollArea *parent)
    : QWidget(parent) {
    setObjectName("filmStripFrames");

    setFixedHeight(100);
    setFixedWidth(parentWidget()->width());
    connect(StopMotion::instance(), &StopMotion::newImageReady, this, &FrameViewer::newImage);
    m_cameraPixmap = QPixmap(":/Resources/camera.svg");
    m_director = Director::instance();
    m_scrollArea = parent;
}

FrameViewer::~FrameViewer() {};

void FrameViewer::paintEvent(QPaintEvent* evt) {
   
    QPainter p(this);
    QRect clipRect = evt->rect();
    m_minFrame = x2index(clipRect.x());
    m_maxFrame = x2index(clipRect.x() + clipRect.width());
    p.fillRect(clipRect, QColor(20, 20, 20));
    bool playing = m_director->getPlaying();

    int frameCount = m_director->getFrameCount();
    int x = currentFrameX();
    std::vector<int> selection = m_director->getFrameSelection();
    int selectionSize = selection.size();
    int cameraFrame = m_director->getCameraFrame();

    if (frameCount > 0) {
        if (m_frameWidth == -1) { getOneFrameWidth(); }
        if (m_frameWidth <= 0) return;
        int height = m_thumbnailRect.height();
        QString previousFrameFile;
        QString nextFrameFile;
        int cameraOffset = 0;
        if (selectionSize > 0 && !playing && m_dragging && m_dropTarget != -1) {
            QRect selected;
            if (m_dropTarget > selection[selectionSize - 1]) {
                selected = QRect(index2x(m_dropTarget) + m_spacing + m_frameWidth / 2, 0, m_frameWidth / 2, this->height());
            }
            else selected = QRect(index2x(m_dropTarget) + m_spacing, 0, m_frameWidth / 2, this->height());
            p.fillRect(selected, QColor(100, 30, 100));
        }
        for (int i = 0; i < frameCount; i++) {
            if (i == cameraFrame) {
                cameraOffset = 1;
            }
            if (i > 0) {
                previousFrameFile = m_director->m_frames.at(i - 1).getFileName();
            }
            if (i < frameCount - 1) {
                nextFrameFile = m_director->m_frames.at(i + 1).getFileName();
            }
            else {
                nextFrameFile = "";
            }
            if (i < m_minFrame || i > m_maxFrame) continue;
            QImage currentThumbnail = m_director->m_frames.at(i).getThumbnail();
            

            // highlight the selected frames
            if (!playing && std::find(selection.begin(), selection.end(), i) != selection.end()) {
                QRect selected(index2x(i + cameraOffset) + m_spacing, 0, m_frameWidth, this->height());
                p.fillRect(selected, QColor(40, 40, 40));
            }
            
            QRectF target(m_spacing + ((i + cameraOffset) * (m_frameWidth + m_spacing)), 10, m_frameWidth, height);
            p.drawImage(target, currentThumbnail, m_thumbnailRect);
            if (m_director->m_frames.at(i).getFileName() == previousFrameFile) {
                p.setPen(Qt::yellow);
                p.drawLine(target.bottomLeft().x() - m_spacing, target.bottomLeft().y(), target.bottomRight().x(), target.bottomRight().y());
                p.setPen(Qt::white);
            }
            else if (m_director->m_frames.at(i).getFileName() == nextFrameFile) {
                p.setPen(Qt::yellow);
                p.drawLine(target.bottomLeft().x(), target.bottomLeft().y(), target.bottomRight().x(), target.bottomRight().y());
                p.setPen(Qt::white);
            }
            
            p.drawText(target.bottomLeft() + QPoint(5, 12), QString::number(i + 1));
            QString soundPath = m_director->m_frames.at(i).getSoundFile();
            
            if (soundPath != "") {
                QImage sound(":/Resources/sound.svg");
                p.drawImage(target.bottomLeft() + QPoint(m_thumbnailRect.width() - 18, 2), sound);
            } 
            if (!playing && m_draggingSound && m_dropTarget == i) {
                QImage sound(":/Resources/yellow_sound.svg");
                p.drawImage(target.bottomLeft() + QPoint(m_thumbnailRect.width() - 18, 2), sound);
            }
            
        }
    }

    // draw the camera
    QRect camRect = m_cameraPixmap.rect();
    int height = camRect.height();
    int width = camRect.width();
    QRectF camTarget;
    if (frameCount > 0) {
        int offset = (m_frameWidth - width) / 2;
        camTarget = QRectF(offset + m_spacing + (cameraFrame * (m_frameWidth + m_spacing)), 10, m_cameraPixmap.width(), 72);
    }
    else {
        camTarget = QRectF(m_spacing, 10, m_cameraPixmap.width(), 72);
    }
    p.drawPixmap(camTarget, m_cameraPixmap, m_cameraPixmap.rect());

    p.setPen(Qt::yellow);
    p.drawLine(x, 0, x, this->height()); 

    QRectF rect = QRectF(x - 4, 0, 9, 10);
    QPainterPath path;
    path.moveTo(rect.left() + (rect.width() / 2), rect.bottom());
    path.lineTo(rect.topLeft());
    path.lineTo(rect.topRight());
    path.lineTo(rect.left() + (rect.width() / 2), rect.bottom());

    p.fillPath(path, QBrush(QColor("yellow")));
    p.setPen(Qt::white);
}

void FrameViewer::mousePressEvent(QMouseEvent* event) {
    int cameraFrame = m_director->getCameraFrame();
    int index = x2index(event->pos().x() - m_spacing);
    if (index < 0) index = 0;
    if (index > m_director->getFrameCount()) index = m_director->getFrameCount();
    bool atCamera = index == cameraFrame;
    QRect soundRect(QPoint(index2x(index) + m_thumbnailRect.width() - 8, 84), QSize(12, 12));
    bool soundClicked = false;
    std::vector<int> selection = m_director->getFrameSelection();
    QPoint testPos = event->pos();
    bool hitBox = soundRect.contains(event->pos());
    //std::string soundPath = m_director->getFrameSoundPath(index).toStdString();
    if (!atCamera && soundRect.contains(event->pos()) && m_director->getFrameSoundPath(index) != "") {
        int i = 3;
        soundClicked = true;
        m_director->playSound(m_director->getOffsetFrame(index));
        //m_director->frameClicked(index, false);
        m_dragging = false;
        m_draggingSound = true;
        m_startIndex = index;
    }
    // first check if moving a selection
    else if (!atCamera && event->modifiers() != Qt::ShiftModifier && std::find(selection.begin(), selection.end(), m_director->getOffsetFrame(index)) != selection.end()) {
        m_dragging = true;
        m_draggingSound = false;
        m_startIndex = index;
    }
    // next check for shift to expand a selection
    else if (!atCamera && event->modifiers() == Qt::ShiftModifier) {
        m_director->frameClicked(index, true);
    }
    // finally just select a frame
    else {
        m_director->frameClicked(index, false);
        m_dragging = true;
        m_startIndex = index;
    }
    //m_currentFrame = index;
    update();
}

void FrameViewer::mouseMoveEvent(QMouseEvent* event) {
    bool atCamera = false;
    int index = -1;
    if (event->buttons() == Qt::LeftButton && (m_draggingSound == true || m_dragging == true)) {
        int cameraFrame = m_director->getCameraFrame();
        m_scrollArea->isVisible();
        index = x2index(event->pos().x() - m_spacing);
        if (index < 0) index = 0;
        if (index > m_director->getFrameCount()) index = m_director->getFrameCount();
        atCamera = index == cameraFrame;
        showFrame(index);
    }
    if (event->buttons() == Qt::LeftButton && m_dragging == true) {
        // first check if moving a selection
        std::vector<int> selection = m_director->getFrameSelection();
        if (!atCamera && index != m_startIndex && std::find(selection.begin(), selection.end(), m_director->getOffsetFrame(index)) == selection.end()) {
            if (index != m_dropTarget) {
                m_dropTarget = index;
                update();
            }
        }
        else {
            if (m_dropTarget != -1) {
                m_dropTarget = -1;
                update();
            }
        }
    }
    else if (event->buttons() == Qt::LeftButton && m_draggingSound == true) {
        if (!atCamera && index != m_startIndex) {
            if (index != m_dropTarget) {
                m_dropTarget = index;
                update();
            }
        }
        else {
            if (m_dropTarget != -1) {
                m_dropTarget = -1;
                update();
            }
        }
    }
}

void FrameViewer::mouseReleaseEvent(QMouseEvent* event) {
    if (m_dragging) {
        int index = x2index(event->pos().x() - m_spacing);
        if (index < 0) index = 0;
        if (index > m_director->getFrameCount()) index = m_director->getFrameCount();
        int cameraFrame = m_director->getCameraFrame();
        bool atCamera = index == cameraFrame;

        // if a drag didn't occur, just select a single frame.
        if (index == m_startIndex) {
            m_director->frameClicked(index, false);
        }
        if (m_dropTarget != -1) {
            m_director->moveFrames(m_dropTarget);
        }
    }
    if (m_draggingSound) {
        int index = x2index(event->pos().x() - m_spacing);
        if (index < 0) index = 0;
        if (index > m_director->getFrameCount()) index = m_director->getFrameCount();
        int cameraFrame = m_director->getCameraFrame();
        bool atCamera = index == cameraFrame;

        if (m_dropTarget != -1) {
            m_director->moveSound(m_startIndex, m_dropTarget);
            update();
        }
    }
    m_dragging = false;
    m_draggingSound = false;
    m_dropTarget = -1;

}

void FrameViewer::contextMenuEvent(QContextMenuEvent* event) {
    int index = x2index(event->x());
    if (index < 0) index = 0;
    if (index > m_director->getFrameCount()) index = m_director->getFrameCount();
    int cameraFrame = m_director->getCameraFrame();
    bool atCamera = index == cameraFrame;
    if (index > m_director->getFrameCount()) return;
    m_director->setLastClicked(Director::FRAME);
    QMenu menu(m_director->getMainWindow());
    if (!atCamera) {
        menu.addAction(Commands::instance()->getAction("Copy"));
        if (Clipboard::instance()->getFrames().size() > 0) {
            menu.addAction(Commands::instance()->getAction("Paste"));
        }
        menu.addSeparator();
        menu.addAction(Commands::instance()->getAction("Duplicate Frame"));
        menu.addSeparator();
        menu.addAction(Commands::instance()->getAction("Delete"));
        menu.addSeparator();
        menu.addAction(Commands::instance()->getAction("Insert Camera"));
    }
    else {
        if (cameraFrame != m_director->getFrameCount()) {
            menu.addAction(Commands::instance()->getAction("Move Camera to the End"));
        }
    }
    menu.exec(event->globalPos());

}

void FrameViewer::updateSize() {
    int frameCount = m_director->getFrameCount();
    if (frameCount > 0) {
        int width = m_director->m_frames.at(0).getThumbnail().width();
        int totalWidth = m_spacing + (frameCount * (width + m_spacing)) + m_cameraPixmap.width() + m_spacing;
        if (totalWidth > parentWidget()->width()) {
            setFixedWidth(totalWidth);
        }
        else setFixedWidth(parentWidget()->width());
    }
    else setFixedWidth(parentWidget()->width());
}

void FrameViewer::newImage() {
    updateSize();
    update();
}

int FrameViewer::getOneFrameWidth() {
    if (m_frameWidth != -1) {
        return m_frameWidth;
    }
    int frameCount = m_director->getFrameCount();
    if (frameCount > 0) {
        m_thumbnailRect = m_director->m_frames.at(0).getThumbnail().rect();
        m_frameWidth = m_thumbnailRect.width();
        return m_frameWidth;
    }
    return m_spacing;
}

int FrameViewer::x2index(int x) {
    return x / (getOneFrameWidth() + m_spacing);
}

int FrameViewer::index2x(int index) {
    return index * (getOneFrameWidth() + m_spacing);
}

int FrameViewer::currentFrameX() {
    return (Director::instance()->getCurrentFrame() * (getOneFrameWidth() + m_spacing)) + m_spacing;
}

void FrameViewer::showFrame(int frame) {
    int width = this->width();
    int x0 = index2x(frame);
    int x1 = getOneFrameWidth() / 2;
    if ((x0 + x1) > this->width()) setFixedWidth(x0 + x1 + x1);
    m_scrollArea->ensureVisible(x0 + x1, 40, x1, 20);
}

//void FrameViewer::setCurrentFrame(int frame) { 
//    m_currentFrame = frame; 
//}