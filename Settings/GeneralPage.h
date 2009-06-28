#ifndef GENERALPAGE_H
#define GENERALPAGE_H
#include <QWidget>

class QComboBox;
class QSpinBox;
class QCheckBox;
class KComboBox;
namespace Settings
{
class SettingsData;

class GeneralPage :public QWidget
{
public:
    GeneralPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );

private:
    KComboBox* _trustTimeStamps;
    QCheckBox* _useEXIFRotate;
    QCheckBox* _useEXIFComments;
    QCheckBox* _searchForImagesOnStart;
    QCheckBox* _skipRawIfOtherMatches;
    QSpinBox* _barWidth;
    QSpinBox* _barHeight;
    QCheckBox* _showSplashScreen;
    QComboBox* _albumCategory;
};
}


#endif /* GENERALPAGE_H */

