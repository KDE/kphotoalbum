#ifndef EXIFPAGE_H
#define EXIFPAGE_H
#include <QComboBox>
#include <QWidget>

namespace Exif { class TreeView; }

namespace Settings
{
class SettingsData;

class ExifPage :public QWidget
{
public:
    ExifPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );

private:
    Exif::TreeView* _exifForViewer;
    Exif::TreeView* _exifForDialog;
    QComboBox* _iptcCharset;
};

}

#endif /* EXIFPAGE_H */

