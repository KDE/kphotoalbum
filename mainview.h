#ifndef MAINVIEW_H
#define MAINVIEW_H
class OptionsDialog;
class ImageConfig;
#include "imageinfo.h"
#include "mainviewui.h"
#include <qdict.h>

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
    void slotSave();
    void slotDeleteSelected();
    void slotSearch();
    void load();
    void slotViewSelected();
    void slotChanges();

protected:
    void configureImages( bool oneAtATime );
    void loadExtraFiles( const QDict<void>& loadedFiles, const QString& indexDirectory, QString directory );
    void load( const QString& indexDirectory,  const QString& filename, QDomElement elm );
    ImageInfoList selected();
    void wellcome();
    virtual void closeEvent( QCloseEvent* e );

private:
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    ImageInfoList _images;
    bool _dirty;
};


#endif /* MAINVIEW_H */

