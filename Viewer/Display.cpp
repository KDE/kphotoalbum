#include "Display.h"
#include <Settings/SettingsData.h>
#include <DB/ImageInfo.h>
Viewer::Display::Display( QWidget* parent, const char* name )
    :QWidget( parent, name ), _info( 0 )
{
}

#include "Display.moc"
