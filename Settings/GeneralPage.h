#ifndef GENERALPAGE_H
#define GENERALPAGE_H
#include <QWidget>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class KComboBox;
class KLineEdit;
namespace Settings
{
class SettingsData;

class GeneralPage :public QWidget
{
public:
    GeneralPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );
    void setUseRawThumbnailSize( const QSize& size );
    QSize useRawThumbnailSize();

private:
    KComboBox* _trustTimeStamps;
    QCheckBox* _useEXIFRotate;
    QCheckBox* _useEXIFComments;
    QCheckBox* _searchForImagesOnStart;
    QCheckBox* _skipRawIfOtherMatches;
    QCheckBox* _useRawThumbnail;
    QSpinBox* _useRawThumbnailWidth;
    QSpinBox* _useRawThumbnailHeight;
    QSpinBox* _barWidth;
    QSpinBox* _barHeight;
    QCheckBox* _showSplashScreen;
    QComboBox* _albumCategory;
    KLineEdit* _excludeDirectories; // Directories to exclude
    QCheckBox* _detectModifiedFiles;
    QLineEdit* _modifiedFileComponent;
    QLineEdit* _originalFileComponent;
    QCheckBox* _moveOriginalContents;
    QCheckBox* _autoStackNewFiles;
    QLineEdit* _copyFileComponent;
    QLineEdit* _copyFileReplacementComponent;
};
}


#endif /* GENERALPAGE_H */

