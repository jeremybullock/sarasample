#ifndef FRAME_H
#define FRAME_H

#include <QObject>
#include <QImage>
#include <memory>

class StopMotion;

class Frame : public QObject
{
    Q_OBJECT

public:
    Frame();
    Frame(const Frame& frame);
    Frame operator = (const Frame& other) {
        if (this == &other)
            return *this;

        m_fileName = other.m_fileName;
        m_hasLiveViewImage = other.m_hasLiveViewImage;
        m_image = other.m_image;
        m_liveViewImage = other.m_liveViewImage;
        m_thumbnail = other.m_thumbnail;
        m_soundFile = other.m_soundFile;
        m_volume = other.m_volume;
        return *this;
    }
    Frame(QImage image, QImage thumbnail, QImage liveViewImage, QString imageNumber);
    Frame(QImage image, QImage thumbnail, QString imageNumber);
    ~Frame();
    QImage getImage() { return m_image; }
    QImage getThumbnail() { return m_thumbnail; }
    QImage getLiveViewImage();
    QString getFileName() { return m_fileName; }
    void setFileName(QString fileName) { m_fileName = fileName; }
    void setSoundFile(QString soundFile) { m_soundFile = soundFile; }
    QString getSoundFile() { return m_soundFile; }

    void setVolume(int volume) { m_volume = volume; }
    int getVolume() { return m_volume; }

private:
    QImage m_thumbnail;
    QImage m_image;
    QImage m_liveViewImage;
    QString m_fileName = "";
    bool m_hasLiveViewImage = false;
    QString m_soundFile = "";
    int m_volume = 50;

};
#endif // FRAME_H
