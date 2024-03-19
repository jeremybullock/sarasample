#ifndef FRAMEVIEWER_H
#define FRAMEVIEWER_H

#include <QObject>
#include <QImage>
#include <QScrollArea>
class StopMotion;
class Frame;
class Director;

class FrameViewer : public QWidget
{
    Q_OBJECT

public:
    FrameViewer(QScrollArea* parent);

    ~FrameViewer();
    void updateSize();
    int getOneFrameWidth();
    void showFrame(int frame);
    //void setTempPlayHead(int frame) { m_tempPlayheadAt = frame; }

private:
    QScrollArea* m_scrollArea;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void newImage();
    int x2index(int x);
    int index2x(int index);
    int currentFrameX();
    QPixmap m_cameraPixmap;
    //int m_tempPlayheadAt = -1;
    int m_spacing = 10;
    Director* m_director;
    bool m_dragging = false;
    bool m_draggingSound = false;
    int m_dropTarget = -1;
    int m_startIndex = -1;
    int m_frameWidth = -1;
    int m_minFrame = -1;
    int m_maxFrame = -1;
    QRect m_thumbnailRect;

};
#endif // FRAMEVIEWER_H
