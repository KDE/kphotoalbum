#ifndef VIEWERPAGE_H
#define VIEWERPAGE_H
#include <QWidget>

class KComboBox;
class QSpinBox;
class QComboBox;
namespace Settings
{
class SettingsData;
class ViewerSizeConfig;

class ViewerPage :public QWidget
{
public:
    ViewerPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );
    void reset(Settings::SettingsData* opt );

private:
    Settings::ViewerSizeConfig* _slideShowSetup;
    Settings::ViewerSizeConfig* _viewImageSetup;
    QComboBox* _smoothScale;
    QSpinBox* _slideShowInterval;
    QSpinBox* _cacheSize;
    KComboBox* _viewerStandardSize;
};


}


#endif /* VIEWERPAGE_H */

