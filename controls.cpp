#include "controls.h"
#include "director.h"
#include "stopmotion/stopmotion.h"
#include "commands.h"

#include <QGridLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QToolButton>
#include <QAction>
#include <QPushButton>
#include <QCommonStyle>
#include <QTimer>


Controls::Controls(QWidget* parent) : QWidget(parent) {
    setContentsMargins(0, 0, 0, 0);
    m_stopMotion = StopMotion::instance();
    setFixedHeight(40);
    m_fpsCombo = new QComboBox(this);
    
    m_captureButton = new QPushButton(tr(""), this);
    m_captureButton->setFixedHeight(36);
    QCommonStyle style;
    m_captureButton->setIcon(QIcon(":/Resources/camera.svg"));
    m_captureButton->setContentsMargins(2, 2, 2, 2);
    m_captureButton->setIconSize(QSize(32, 32));

    m_onionOpacityFld = new QSlider(Qt::Horizontal, this);
    m_onionOpacityFld->setRange(0, 100);
    m_onionOpacityFld->setValue(100);
    m_onionOpacityFld->setDisabled(false);
    m_onionOpacityFld->setFixedWidth(100);

    QStringList fps;
    fps << "12" << "15" << "20" << "24" << "30";
    m_fpsCombo->addItems(fps);
    QGridLayout *mainLayout = new QGridLayout(this); 
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* leftLayout = new QHBoxLayout(this);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* centerLayout = new QHBoxLayout(this);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* rightLayout = new QHBoxLayout(this);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->addStretch();

    QToolButton* playButton = new QToolButton(this);
    playButton->setDefaultAction(Commands::instance()->getAction("Play")); 
    playButton->setIconSize(QSize(25, 25));
    leftLayout->addWidget(playButton);

    QToolButton* loopButton = new QToolButton(this);
    loopButton->setDefaultAction(Commands::instance()->getAction("Loop"));
    loopButton->setIconSize(QSize(25, 25));
    leftLayout->addWidget(loopButton);

    QToolButton* shortPlayButton = new QToolButton(this);
    shortPlayButton->setDefaultAction(Commands::instance()->getAction("Short Play"));
    shortPlayButton->setIconSize(QSize(25, 25));
    leftLayout->addWidget(shortPlayButton);

    leftLayout->addWidget(m_fpsCombo);
    
    leftLayout->addWidget(new QLabel("          "));
    m_rotButton = new QToolButton(this);
    m_rotButton->setDefaultAction(Commands::instance()->getAction("Rule of Thirds"));
    m_rotButton->setIconSize(QSize(26, 25));
    leftLayout->addWidget(m_rotButton);

    m_overlayButton = new QToolButton(this);
    m_overlayButton->setDefaultAction(Commands::instance()->getAction("Overlays"));
    m_overlayButton->setIconSize(QSize(26, 25));
    leftLayout->addWidget(m_overlayButton);

    QToolButton* flipButton = new QToolButton(this);
    flipButton->setDefaultAction(Commands::instance()->getAction("Flip Horizontally"));
    flipButton->setIconSize(QSize(26, 25));
    leftLayout->addWidget(flipButton);

    QToolButton* resetButton = new QToolButton(this);
    resetButton->setDefaultAction(Commands::instance()->getAction("Reset Viewer Position"));
    resetButton->setIconSize(QSize(34, 25));
    leftLayout->addWidget(resetButton);

    leftLayout->addWidget(new QLabel("      "));

    QToolButton* soundButton = new QToolButton(this);
    soundButton->setDefaultAction(Commands::instance()->getAction("Toggle Sound"));
    soundButton->setIconSize(QSize(25, 25));
    leftLayout->addWidget(soundButton);

    centerLayout->addStretch();
    centerLayout->addWidget(m_captureButton, 0);
    centerLayout->addStretch();

    rightLayout->addWidget(m_onionOpacityFld, 1);
    rightLayout->addWidget(new QLabel("     "));

    m_focusButton = new QToolButton(this);
    m_focusButton->setDefaultAction(Commands::instance()->getAction("Focus"));
    m_focusButton->setIconSize(QSize(25, 25));
    rightLayout->addWidget(m_focusButton);

    m_pickFocusButton = new QToolButton(this);
    m_pickFocusButton->setDefaultAction(Commands::instance()->getAction("Pick Focus"));
    m_pickFocusButton->setIconSize(QSize(25, 25));
    rightLayout->addWidget(m_pickFocusButton);

    m_highQualityButton = new QToolButton(this);
    m_highQualityButton->setDefaultAction(Commands::instance()->getAction("High Quality"));
    m_highQualityButton->setIconSize(QSize(25, 25));
    rightLayout->addWidget(m_highQualityButton);

    mainLayout->addLayout(leftLayout, 0, 0, Qt::AlignLeft);
    mainLayout->addLayout(centerLayout, 0, 0, Qt::AlignCenter);
    mainLayout->addLayout(rightLayout, 0, 0, Qt::AlignRight);

    setLayout(mainLayout);

    connect(m_fpsCombo, &QComboBox::currentTextChanged, Director::instance(), &Director::onFpsChanged);
    connect(Director::instance(), &Director::fpsChanged, this, &Controls::updateFps);
    connect(m_onionOpacityFld, SIGNAL(valueChanged(int)), this,
        SLOT(onOnionOpacitySliderChanged(int)));
    connect(Director::instance(), SIGNAL(opacityChanged(double)), this,
        SLOT(onOpacityChanged(double)));
    connect(m_captureButton, SIGNAL(clicked(bool)), this,
        SLOT(onCaptureButtonClicked(bool)));
    connect(StopMotion::instance()->m_countdownTimer, SIGNAL(timeout()), this,
        SLOT(onIntervalCountDownTimeout()));
    connect(m_stopMotion->m_intervalTimer, SIGNAL(timeout()), this,
        SLOT(onIntervalCaptureTimerTimeout()));
    connect(m_stopMotion, SIGNAL(intervalToggled(bool)), this,
        SLOT(onIntervalToggled(bool)));
    connect(m_stopMotion, SIGNAL(intervalStarted()), this,
        SLOT(onIntervalStarted()));
    connect(m_stopMotion, SIGNAL(intervalStopped()), this,
        SLOT(onIntervalStopped()));
    connect(m_stopMotion, SIGNAL(newCameraSelected(int, bool)), this,
        SLOT(onNewCameraSelected(int, bool)));


    m_onionOpacityFld->setValue((double)100 * Director::instance()->getOpacity());
}

Controls::~Controls() {};

int Controls::getFps() {
    return m_fpsCombo->currentText().toInt();
}

//-----------------------------------------------------------------------------

void Controls::updateFps() {
    m_fpsCombo->blockSignals(true);
    m_fpsCombo->setCurrentText(QString::number(Director::instance()->getFps()));
    m_fpsCombo->blockSignals(false);
}

//-----------------------------------------------------------------------------

void Controls::onOnionOpacityFldEdited() {
    double value = (double)m_onionOpacityFld->value() / 100.0;
    Director::instance()->setOpacity(value);
}

//-----------------------------------------------------------------------------

void Controls::onOnionOpacitySliderChanged(int ignore) {
    double value = (double)m_onionOpacityFld->value() / 100.0;
    Director::instance()->setOpacity(value);
}

//-----------------------------------------------------------------------------

void Controls::onOpacityChanged(double opacity) {
    m_onionOpacityFld->blockSignals(true);
    m_onionOpacityFld->setValue(100.0 * opacity);
    m_onionOpacityFld->blockSignals(false);
}

//-----------------------------------------------------------------------------

void Controls::showEvent(QShowEvent* event) {
    bool hasCanon = false;
    bool hasWebcam = false;

    m_stopMotion->m_canon->initializeCanonSDK();
    if (!m_stopMotion->m_canon->m_sessionOpen) {
        m_focusButton->hide();
        m_pickFocusButton->hide();
        m_highQualityButton->hide();
    }
    else {
        m_focusButton->show();
        m_pickFocusButton->show();
        m_highQualityButton->show();
    }
}

//-----------------------------------------------------------------------------

void Controls::onNewCameraSelected(int index, bool useWebcam) {

    if (index == 0) {
        m_focusButton->hide();
        m_pickFocusButton->hide();
        m_highQualityButton->hide();
    }
    else if (useWebcam) {
        m_focusButton->hide();
        m_pickFocusButton->hide();
        m_highQualityButton->hide();
    }
    else {
        m_focusButton->show();
        m_pickFocusButton->show();
        m_highQualityButton->show();
    }
}

//-----------------------------------------------------------------------------

void Controls::hideEvent(QHideEvent* event) {
    // stop interval timer if it is active
    if (m_stopMotion->m_intervalStarted && m_captureButton->isChecked()) {
        m_captureButton->setChecked(false);
        onCaptureButtonClicked(false);
    }
}

//-----------------------------------------------------------------------------

void Controls::onIntervalCountDownTimeout() {
    m_captureButton->setText(QString::number(
        m_stopMotion->m_intervalTimer->isActive()
        ? (m_stopMotion->m_intervalTimer->remainingTime() / 1000 + 1)
        : 0));
}

//-----------------------------------------------------------------------------
void Controls::onIntervalToggled(bool on) {
    m_captureButton->setCheckable(on);
    if (on)
        m_captureButton->setText(tr("Start Capturing"));
    else
        m_captureButton->setText(tr(""));
}

//-----------------------------------------------------------------------------
void Controls::onIntervalStarted() {
    m_captureButton->setText(tr("Stop Capturing"));
    m_captureButton->blockSignals(true);
    m_captureButton->setChecked(true);
    m_captureButton->blockSignals(false);
}

//-----------------------------------------------------------------------------
void Controls::onIntervalStopped() {
    m_captureButton->setText(tr("Start Capturing"));
    m_captureButton->blockSignals(true);
    m_captureButton->setChecked(false);
    m_captureButton->blockSignals(false);
}

//-----------------------------------------------------------------------------

void Controls::onCaptureButtonClicked(bool on) {
    if (m_stopMotion->m_isTimeLapse) {
        // start interval capturing
        if (on) {
            m_stopMotion->startInterval();
        }
        // stop interval capturing
        else {
            m_stopMotion->stopInterval();
        }
    }
    // capture immediately
    else {
        m_stopMotion->captureImage();
    }
}

//-----------------------------------------------------------------------------

void Controls::onIntervalCaptureTimerTimeout() {
    if (m_stopMotion->m_liveViewStatus < 1) {
        onCaptureButtonClicked(false);
    }
}