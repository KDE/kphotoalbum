#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <qdialog.h>
class QLineEdit;

class WelComeDialog : public QDialog
{
    Q_OBJECT

public:
    WelComeDialog( QWidget* parent = 0, const char* name = 0 );
    QString configFileName() const;

protected slots:
    void slotLoadDemo();
    void createSetup();
private:
    QString _configFile;
};


class FileDialog : public QDialog
{
    Q_OBJECT
public:
    FileDialog( QWidget* parent, const char* name = 0 );
    QString getFileName();
protected slots:
    void slotBrowseForDirecory();
private:
    QLineEdit* _lineEdit;
};

#endif // WELCOMEDIALOG_H
