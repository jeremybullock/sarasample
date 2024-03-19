#include "timeline.h"
#include "frame.h"
#include "frameviewer.h"
#include "stopmotion/stopmotion.h"
#include "director.h"
#include <QBoxLayout>;
#include <QScrollArea>
#include <QScrollBar>


Timeline::Timeline(QWidget* parent) : QWidget(parent) {
    //setFixedHeight(120);
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_mainLayout);
    
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->horizontalScrollBar()->setObjectName("LevelStripScrollBar");
    //m_scrollArea->setFixedHeight(100);
    
    m_frameViewer = new FrameViewer(m_scrollArea);
    m_frameViewer->setStyleSheet("background-color: #fff;");
    m_scrollArea->setWidget(m_frameViewer);
    m_scrollArea->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_scrollArea);

    //connect(StopMotion::instance(), &StopMotion::newImageReady, this, &Timeline::addNewThumbnail);
}

Timeline::~Timeline() {};


void Timeline::resizeEvent(QResizeEvent*) {
    m_frameViewer->updateSize();
}
