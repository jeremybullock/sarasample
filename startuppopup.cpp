#include "startuppopup.h"
#include "stopmotion/stopmotion.h"
#include "newscenepopup.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileDialog>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QLabel>
#include <QFile>
#include <QGroupBox>


void removeAll(QLayout* layout) {
    QLayoutItem* child;
    while (layout->count() != 0) {
        child = layout->takeAt(0);
        if (child->layout() != 0) {
            removeAll(child->layout());
        }
        else if (child->widget() != 0) {
            delete child->widget();
        }
        delete child;
    }
}


std::pair<QString, QString> StartupPopup::openFile() {
    bool validFile = false;
    QString filePath;
    QString fileNameShort;
    std::pair<QString, QString> emptyStrings;
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        "Open Project", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), "Arlo Files (*.arlo)");
    if (fileName.size() <= 0) {
        return  emptyStrings;
    }
    QFile file(fileName);
    if (!file.exists()) {
        QMessageBox msgBox;
        msgBox.setText("Please choose a project file.");
        int value = msgBox.exec();
        return emptyStrings;
    }
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&file);
    xmlReader.readNext();
    while (!xmlReader.atEnd()) {
        if (xmlReader.isStartElement()) {
            if (xmlReader.name().toString() == "Webcam") {
                validFile = true;
                QFileInfo fileInfo(fileName);
                filePath = fileInfo.absoluteDir().absolutePath();
                fileNameShort = fileInfo.baseName();
                auto pair = std::make_pair(filePath, fileNameShort);
                return pair;
                break;
            }
        }
        xmlReader.readNext();
    }
    if (!validFile) {
        QMessageBox msgBox;
        msgBox.setText("Please choose a valid project file.");
        int value = msgBox.exec();
        return emptyStrings;
    }
}

StartupPopup::StartupPopup(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Arlo");
    setFixedWidth(600);
    setFixedHeight(300);

    QHBoxLayout* outsideLayout = new QHBoxLayout(this);
    QImage logo = QImage(":/Resources/logo.svg");
    logo = logo.scaled(QSize(256, 256));
    QLabel* logoLabel = new QLabel(this);
    logoLabel->setPixmap(QPixmap::fromImage(logo));
    logoLabel->setFixedSize(QSize(256, 256));
    outsideLayout->setContentsMargins(40, 20, 40, 40);
    outsideLayout->addWidget(logoLabel);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QPushButton* newProjectButton = new QPushButton("New Project");
    QPushButton* loadProjectButton = new QPushButton("Open Project");
    QPushButton* recoverProjectButton = new QPushButton("Recover Project");

    QHBoxLayout* openLayout = new QHBoxLayout();
    openLayout->addStretch();
    openLayout->addWidget(loadProjectButton);
    openLayout->addStretch(); 

    QHBoxLayout *createLayout = new QHBoxLayout();
    createLayout->addStretch();
    createLayout->addWidget(newProjectButton);
    createLayout->addStretch();

    QHBoxLayout* recoverLayout = new QHBoxLayout();
    recoverLayout->addStretch();
    recoverLayout->addWidget(recoverProjectButton);
    recoverLayout->addStretch();

    m_recentBox = new QGroupBox(tr("Recent Scenes"), this);
    m_recentBox->setObjectName("SolidLineFrame");
    m_recentBox->setContentsMargins(10, 10, 10, 10);
    m_recentBox->setFixedWidth(256);
    m_recentBox->setFixedHeight(150);
    m_recentSceneLay = new QVBoxLayout();
    m_recentSceneLay->setContentsMargins(5, 5, 5, 5);
    m_recentSceneLay->setSpacing(2);
    m_recentBox->setLayout(m_recentSceneLay);

    vLayout->addWidget(m_recentBox);

    vLayout->addStretch();
    vLayout->addLayout(createLayout);
    vLayout->addStretch();
    vLayout->addLayout(openLayout);
    vLayout->addStretch();
    vLayout->addLayout(recoverLayout);
    vLayout->addStretch();

    outsideLayout->addLayout(vLayout);
    setLayout(outsideLayout);


    setModal(true);
    setResult(QDialog::Rejected);
    connect(loadProjectButton, &QPushButton::pressed, this, &StartupPopup::openButtonPressed);
    connect(newProjectButton, &QPushButton::pressed, this, &StartupPopup::newButtonPressed);
    connect(recoverProjectButton, &QPushButton::pressed, this, &StartupPopup::recoverButtonPressed);
    refreshRecentScenes();
}

StartupPopup::~StartupPopup() {};

void StartupPopup::openButtonPressed() {
    std::pair<QString, QString> pair = openFile();
    if (pair.first.size() > 0 && pair.second.size() > 0) {
        m_filePath = pair.first;
        m_fileName = pair.second;
        done(2);
    }
}

void StartupPopup::newButtonPressed() {
    NewScenePopup newPopup;
    if (newPopup.exec()) {
        m_fileName = newPopup.m_fileName;
        m_filePath = newPopup.m_filePath;
        accept();
    }
}

void StartupPopup::recoverButtonPressed() {
    QString folder = QFileDialog::getExistingDirectory(this, "Choose the Project Folder", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if (folder.size() > 0) {
        QDir dir(folder);
        if (!dir.exists()) return;
        std::string folderStr = folder.toStdString();
        QString imagesPath = folder + "/Images";
        QString liveViewPath = folder + "/LiveView";
        QString thumbPath = folder + "/Thumbs";
        QDir images(imagesPath);
        QDir liveView(liveViewPath);
        QDir thumbs(thumbPath);
        bool exists = true;
        if (!images.exists() || !liveView.exists() || !thumbs.exists()) {
            QMessageBox msgBox;
            msgBox.setText("The selected folder is missing some required subfolders.\nUnable to recover from the selected folder.");
            int value = msgBox.exec();
            return;
        }
        QStringList arloList = dir.entryList(QStringList() << "*.arlo", QDir::Files);
        if (arloList.size() > 0) {
            QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Arlo",
                tr("There is already an Arlo project file in the folder.\nDo you want to continue?\n"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
            if (resBtn == QMessageBox::No) {
                return;
            }
        }      
        QStringList imageList = images.entryList(QStringList() << "*.jpg", QDir::Files, QDir::Name);
        m_tempFrames.clear();
        bool first = true;
        bool webcam = false;
        //int width = 0;
        //int height = 0;
        for (auto imageFile : imageList) {
            bool allThere = true;
            if (!QFileInfo(imagesPath + "/" + imageFile).exists()) allThere = false;
            if (!QFileInfo(thumbPath + "/" + imageFile).exists()) allThere = false;
            if (!QFileInfo(liveViewPath + "/" + imageFile).exists()) allThere = false;
            if (!allThere) continue;
            
            if (first) {
                webcam = !QFileInfo(folder + "/FullRes/" + imageFile).exists();
            }
            first = false;

            Frame frame;
            frame.setFileName(imageFile);
            //if (frame.getFileName() == "") continue;
            m_tempFrames.push_back(frame);
        }
        bool success = false;
        if (m_tempFrames.size() > 0) {
            success = saveXml(folder + "/" + QFileInfo(folder).baseName() + ".arlo", webcam);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Unable to recover a project from the folder.");
            int value = msgBox.exec();
            return;
        }
        if (success) {
            QMessageBox msgBox;
            msgBox.setText("Successfully created a project file with " + QString::number(m_tempFrames.size()) + " frames.");
            int value = msgBox.exec();
            m_filePath = folder;
            m_fileName = QFileInfo(folder).baseName();
            done(2);
        }
        else {
            QMessageBox msgBox;
            msgBox.setText("Unable to save the new project file.\nCheck the the location is writable and try again.");
            int value = msgBox.exec();
            return;
        }
    }
}

//-----------------------------------------------------------------------------

void StartupPopup::refreshRecentScenes() {
    removeAll(m_recentSceneLay);    
    loadRecentFiles();
    m_recentNamesLabels.clear();
    m_recentNamesLabels = std::vector<StartupLabel*>(m_sceneNames.size());

    if (m_sceneNames.size() <= 0) {
        m_recentSceneLay->addWidget(new QLabel(tr("No Recent Scenes"), this), 1,
            Qt::AlignTop);
    }
    else {
        int i = 0;
        for (QString name : m_sceneNames) {
            if (i > 4) break;  // box can hold 5 scenes
            QString baseName = QFileInfo(name).baseName();
         
            m_recentNamesLabels[i] = new StartupLabel(baseName, this, i);
            m_recentNamesLabels[i]->setToolTip(name);
            m_recentSceneLay->addWidget(m_recentNamesLabels[i], 0, Qt::AlignTop);
            i++;
        }
    }
    bool ret = true;
    for (int i = 0;
        i < m_recentNamesLabels.size() && i < RECENT_SCENES_MAX_COUNT; i++) {
        ret = ret && connect(m_recentNamesLabels[i], SIGNAL(wasClicked(int)), this,
            SLOT(onRecentSceneClicked(int)));
    }
    assert(ret);
    m_recentSceneLay->addStretch(1);
}

//-----------------------------------------------------------------------------
 
bool StartupPopup::checkValidFile(QString fileName) {
    QFile file(fileName);
    if (!file.exists()) {
        return false;
    }


    file.open(QIODevice::ReadOnly);
    bool validFile = false;
    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&file);
    xmlReader.readNext();
    while (!xmlReader.atEnd()) {
        if (xmlReader.isStartElement()) {
            if (xmlReader.name().toString() == "Webcam") {
                validFile = true;
                break;
            }
        }
        xmlReader.readNext();
    }
    file.close();
    return validFile;
}
 
//-----------------------------------------------------------------------------

void StartupPopup::onRecentSceneClicked(int index) {
    if (index < 0) return;
    if (index > m_sceneNames.size()) return;
    QString fileName = m_sceneNames[index];
    bool exists = QFileInfo(fileName).exists();
    if (exists) {
        bool validFile = checkValidFile(fileName);
        if (validFile) {
            QFileInfo fileInfo(fileName);
            QString filePath = fileInfo.absoluteDir().absolutePath();
            QString fileName = fileInfo.baseName();
            m_filePath = filePath;
            m_fileName = fileName;
            done(2);
        }
    }
}

//-----------------------------------------------------------------------------

void StartupPopup::loadRecentFiles() {
    m_sceneNames.clear();

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString empty;
    std::string tempStr = path.toStdString();

    QString tempFile = path + "/recentFiles.xml";
    QString xmlFileName = tempFile;
    QFile xmlFile(xmlFileName);
    if (!xmlFile.exists()) return;
    xmlFile.open(QIODevice::ReadOnly);

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&xmlFile);

    xmlReader.readNext();
    while (!xmlReader.atEnd()) {
        if (xmlReader.name().toString().contains("File_")) {
            QString text = xmlReader.readElementText();
            m_sceneNames.push_back(text);
        }
        xmlReader.readNext();
    }
}

//-----------------------------------------------------------------------------

bool StartupPopup::saveXml(QString filePath, bool webcam) {
    if (QFileInfo(filePath).exists()) {
        bool success = QFile::remove(filePath);
        if (!success) return false;
    }
    QFile xmlFile(filePath);

    if (!xmlFile.open(QIODevice::WriteOnly)) return false;
    QXmlStreamWriter xmlWriter(&xmlFile);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("body");

    xmlWriter.writeStartElement("SceneInfo");
    xmlWriter.writeTextElement("LevelName", QFileInfo(filePath).baseName());
    xmlWriter.writeTextElement("CameraFrame", QString::number(m_tempFrames.size()));
    xmlWriter.writeTextElement("CurrentFrame", QString::number(m_tempFrames.size()));
    xmlWriter.writeTextElement("FrameRate", QString::number(12));
    xmlWriter.writeTextElement("Opacity", QString::number(0.7));
    xmlWriter.writeTextElement("BlackScreen", "false");
    xmlWriter.writeTextElement("UseScreen1", "false");
    xmlWriter.writeTextElement("UseScreen2", "false");
    xmlWriter.writeTextElement("UseScreen3", "false");
    xmlWriter.writeTextElement("Screen1Color", "#000000");
    xmlWriter.writeTextElement("Screen2Color", "#000000");
    xmlWriter.writeTextElement("Screen3Color", "#000000");
    xmlWriter.writeTextElement("ReviewTime", QString::number(0));
    xmlWriter.writeTextElement("Brightness", QString::number(0));
    xmlWriter.writeTextElement("TimeLapse", "false");
    xmlWriter.writeTextElement("TimeLapseInterval", QString::number(10));
    xmlWriter.writeTextElement("OverlayType", QString::number(0));
    xmlWriter.writeTextElement("OverlayOpacity", QString::number(100));
    xmlWriter.writeTextElement("RuleOfThirds", QString::number(0));
    xmlWriter.writeTextElement("Overlays", QString::number(0));
    xmlWriter.writeTextElement("Flip", QString::number(0));
    xmlWriter.writeEndElement(); // scene info

    xmlWriter.writeStartElement("CameraInfo");
        xmlWriter.writeTextElement("Webcam", webcam ? "yes" : "no");


    xmlWriter.writeEndElement(); // camera info

    xmlWriter.writeStartElement("Images");
    int i = 0;
    for (auto frame : m_tempFrames) {
        QString fileName = frame.getFileName();
        //QString number = QStringLiteral("%1").arg(frameNumber, 4, 10, QLatin1Char('0'));
        //number = m_levelName + "_" + number + ".jpg";
        xmlWriter.writeStartElement("Image_" + QString::number(i));
        xmlWriter.writeTextElement("Frame_" + QString::number(i), fileName);
        xmlWriter.writeTextElement("SoundFile", "");
        i++;
        xmlWriter.writeEndElement(); // end of image
    }
    xmlWriter.writeEndElement(); // Images

    xmlWriter.writeStartElement("Drawings");

    xmlWriter.writeEndElement(); // Drawings

    xmlWriter.writeEndElement(); // body
    xmlWriter.writeEndDocument();
    xmlFile.close();

    return true;
}

//-----------------------------------------------------------------------------

StartupLabel::StartupLabel(const QString& text, QWidget* parent, int index)
    : QLabel(parent), m_index(index) {
    setText(text);
    setObjectName("StartupLabel");
}

//-----------------------------------------------------------------------------

StartupLabel::~StartupLabel() {}

//-----------------------------------------------------------------------------

void StartupLabel::mousePressEvent(QMouseEvent* event) {
    m_text = text();
    std::string strText = m_text.toStdString();
    emit wasClicked(m_index);
}

