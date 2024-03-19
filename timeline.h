#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>

class QHBoxLayout;
class StopMotion;
class QScrollArea;
class FrameViewer;

class Timeline : public QWidget
{
    Q_OBJECT

public:
    Timeline(QWidget* parent = nullptr);
    ~Timeline();
    FrameViewer* getFrameViewer() { return m_frameViewer; }

private:
    QHBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    FrameViewer* m_frameViewer;
    void resizeEvent(QResizeEvent*) override;


//private slots:
//    void addNewThumbnail();

};
#endif // TIMELINE_H
