#ifndef MAINVIEW_H
#define MAINVIEW_H
class OptionsDialog;
class ImageConfig;
#include "imageinfo.h"
#include "mainviewui.h"

class MainView :public MainViewUI
{
    Q_OBJECT

public:
    MainView( QWidget* parent,  const char* name = 0 );

protected:
    void slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void save();

protected slots:
    void imageDeleted( QObject* );

private:
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    ImageInfoList _images;
};


#endif /* MAINVIEW_H */

