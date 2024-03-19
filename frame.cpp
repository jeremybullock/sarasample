#include "frame.h"
#include "stopmotion/stopmotion.h"
#include <QImage>

Frame::Frame() {
}

Frame::Frame(const Frame& frame) {
    m_image = frame.m_image;
    m_thumbnail = frame.m_thumbnail;
    m_fileName = frame.m_fileName;
    m_liveViewImage = frame.m_liveViewImage;
    m_hasLiveViewImage = frame.m_hasLiveViewImage;
    m_soundFile = frame.m_soundFile;
    m_volume = frame.m_volume;
}


Frame::Frame(QImage image, QImage thumbnail, QImage liveViewImage, QString fileName) {
    m_image = image;
    m_thumbnail = thumbnail;
    m_liveViewImage = liveViewImage;
    m_hasLiveViewImage = true;
    m_fileName = fileName;
}

Frame::Frame(QImage image, QImage thumbnail, QString fileName) {
    m_image = image;
    m_thumbnail = thumbnail;
    m_fileName = fileName;
}

Frame::~Frame() {};
    
QImage Frame::getLiveViewImage() { 
    if (!m_hasLiveViewImage) {
        return m_image;
    }
    return m_liveViewImage; 
}