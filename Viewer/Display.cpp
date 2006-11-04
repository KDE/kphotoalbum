#include "Display.h"
#include <Settings/SettingsData.h>
#include <DB/ImageInfo.h>
Viewer::Display::Display( QWidget* parent, const char* name )
    :QWidget( parent, name ), _info( 0 )
{
}

void Viewer::Display::zoomStandard()
{
    switch (Settings::SettingsData::instance()->viewerStandardSize()) {
    case Settings::NaturalSize:
        zoomPixelForPixel();
        break;
    case Settings::NaturalSizeIfFits:
        if (_info->size().width() <= width() &&
            _info->size().height() <= height())
            zoomPixelForPixel();
        else
            zoomFull();
        break;
    case Settings::FullSize:
    default:
        zoomFull();
        break;
    }
}

#include "Display.moc"
