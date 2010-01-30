#ifndef FILEDETECTION_H
#define FILEDETECTION_H
#include <QWidget>

class KComboBox;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
namespace Settings
{
class SettingsData;

class FileVersionDetectionPage :public QWidget
{
public:
    FileVersionDetectionPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );

private:
    QCheckBox* _detectModifiedFiles;
    QLineEdit* _modifiedFileComponent;
    QLineEdit* _originalFileComponent;
    QCheckBox* _moveOriginalContents;
    QCheckBox* _autoStackNewFiles;
    QLineEdit* _copyFileComponent;
    QLineEdit* _copyFileReplacementComponent;
};


}


#endif /* FILEDETECTION_H */

