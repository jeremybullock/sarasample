#ifndef NEWSCENEPOPUP_H
#define NEWSCENEPOPUP_H

#include <QDialog>

class QLineEdit;
class QLabel;

class NewScenePopup : public QDialog
{
    Q_OBJECT

public:
    NewScenePopup(QWidget* parent = nullptr);

    ~NewScenePopup();
    QString m_fileName;
    QString m_filePath;

private:
    QLineEdit* m_nameEdit;
    QLabel* m_folderLabel;

    void chooseFolderPressed();
    void createButtonPressed();
};
#endif // NEWSCENEPOPUP_H
