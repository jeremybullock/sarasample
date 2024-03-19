#include "newscenepopup.h"

#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpressionValidator>

NewScenePopup::NewScenePopup(QWidget *parent) : QDialog(parent) {
    setWindowTitle("New Scene");

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(10);
    m_nameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z0-9_]+"), this));
    m_folderLabel = new QLabel("", this);
    m_folderLabel->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    QPushButton* chooseFolderButton = new QPushButton("Choose Folder", this);
    QPushButton* okButton = new QPushButton("Create", this);
    okButton->setDefault(true);

    QGridLayout* mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    mainLayout->addWidget(new QLabel("Project Name: ", this), 0, 0);
    mainLayout->addWidget(m_nameEdit, 0, 1, 1, 2);

    mainLayout->addWidget(new QLabel("Folder: ", this), 1, 0);
    mainLayout->addWidget(m_folderLabel, 1, 1);
    mainLayout->addWidget(chooseFolderButton, 1, 2);
    mainLayout->addWidget(okButton, 2, 2);
    mainLayout->setColumnMinimumWidth(1, 150);

    setModal(true);
    setResult(QDialog::Rejected);
    connect(chooseFolderButton, &QPushButton::pressed, this, &NewScenePopup::chooseFolderPressed);
    connect(okButton, &QPushButton::pressed, this, &NewScenePopup::createButtonPressed);
}

NewScenePopup::~NewScenePopup() {};

void NewScenePopup::chooseFolderPressed() {
    QString folder = QFileDialog::getExistingDirectory();
    if (folder.size() > 0) {
        m_folderLabel->setText(folder);
    }
}

void NewScenePopup::createButtonPressed() {
    if (m_folderLabel->text().size() < 1) {
        QMessageBox msgBox;
        msgBox.setText("Please choose a folder.");
        int value = msgBox.exec();
        return;
    }
    QDir folder(m_folderLabel->text());
    if (!folder.exists()) {
        QMessageBox msgBox;
        msgBox.setText("Please choose an existing folder.");
        int value = msgBox.exec();
        return;
    }
    if (m_nameEdit->text().size() < 1) {
        QMessageBox msgBox;
        msgBox.setText("Please enter a project name.");
        int value = msgBox.exec();
        return;
    }
    m_fileName = m_nameEdit->text();
    m_filePath = m_folderLabel->text() + "/" + m_fileName;
    QDir dir(m_filePath);
    if (dir.exists()) {
        if (!dir.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText("The specified project folder isn't empty.  \nPlease choose a new name.");
            int value = msgBox.exec();
            return;
        }
    }
    if (!dir.exists()) {
        dir.mkpath(".");
        if (!dir.exists()) {
            QMessageBox msgBox;
            msgBox.setText("An error occurred. Please make sure the project folder is writeable.");
            int value = msgBox.exec();
            return;
        }
    }
    accept();
}
    
