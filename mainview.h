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

protected slots:
    void slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void save();
    void slotDeleteSelected();
    void slotSearch();

protected:
    void configureImages( bool oneAtATime );

private:
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    ImageInfoList _images;
    ImageInfoList _curView;
};


#endif /* MAINVIEW_H */

