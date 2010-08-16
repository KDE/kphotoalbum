#ifndef THUMBNAILPAGE_H
#define THUMBNAILPAGE_H
#include <QWidget>

class KColorButton;
class QLabel;
class QCheckBox;
class KComboBox;
class QSpinBox;
namespace Settings {
class SettingsData;

class ThumbnailsPage :public QWidget
{
public:
    ThumbnailsPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );
    bool thumbnailSizeChanged( Settings::SettingsData* opt ) const;

private:
    QSpinBox* _previewSize;
    QSpinBox* _thumbnailSize;
    KComboBox* _thumbnailAspectRatio;
    QSpinBox* _thumbnailSpace;
    QCheckBox* _thumbnailDarkBackground;
    QCheckBox* _thumbnailDisplayGrid;
    QCheckBox* _displayLabels;
    QCheckBox* _displayCategories;
    QSpinBox* _autoShowThumbnailView;
    QLabel* _thumbnailMegabyteInfo;
    KColorButton* _backgroundColor;
};

}


#endif /* THUMBNAILPAGE_H */

