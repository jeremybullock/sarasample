#ifndef CONTROLS_H
#define CONTROLS_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>

class QPushButton;
class StopMotion;
class QSlider;
class StopMotion;
class QToolButton;

class Controls : public QWidget
{
    Q_OBJECT

public:
    Controls(QWidget* parent = nullptr);
    ~Controls();
    int getFps();

protected:
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    QComboBox* m_fpsCombo;
    QSlider* m_onionOpacityFld;
    QPushButton* m_captureButton;
    StopMotion* m_stopMotion;
    QToolButton* m_focusButton;
    QToolButton* m_pickFocusButton;
    QToolButton* m_highQualityButton;
    QToolButton* m_rotButton;
    QToolButton* m_overlayButton;
    
protected slots:
    void onOnionOpacityFldEdited();
    void onOnionOpacitySliderChanged(int ignore);
    void onOpacityChanged(double opacity);
    void onIntervalCountDownTimeout();
    void onIntervalToggled(bool);
    void onIntervalStarted();
    void onIntervalStopped();
    void onCaptureButtonClicked(bool);
    void onIntervalCaptureTimerTimeout();
    void onNewCameraSelected(int, bool);
    void updateFps();

signals:
    void playPressed();
};
#endif // CONTROLS_H
