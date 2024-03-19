#pragma once

#ifndef JPGCONVERTER_H
#define JPGCONVERTER_H

//#include "opencv2/opencv.hpp"
#include "turbojpeg.h"

// Canon Includes
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"

#include <QObject>
#include <QThread>
#include <QImage>

class QCamera;
class QCameraInfo;

//=============================================================================
// JpgConverter
//-----------------------------------------------------------------------------

class JpgConverter : public QThread {
  Q_OBJECT

  EdsStreamRef m_stream;

  QImage m_finalImage;
  // bool m_scale     = false;
  int m_scaleWidth = 0;

public:
  JpgConverter();
  ~JpgConverter();
  static void saveJpg(QImage, QString path);
  static bool loadJpg(QString path, QImage& image);

  void setStream(EdsStreamRef stream);
  // void setScale(bool scale) { m_scale = scale; }
  void setScaleWidth(bool scaleWidth) { m_scaleWidth = scaleWidth; }
  QImage getImage() { return m_finalImage; }
  void convertFromJpg();

protected:
  void run() override;

signals:
  void imageReady(bool);
};

//#endif

#endif  // JPGCONVERTER_H