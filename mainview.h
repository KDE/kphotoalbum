#ifndef MAINVIEW_H
#define MAINVIEW_H
class OptionsDialog;
class ImageConfig;
#include "imageinfo.h"
#include "mainviewui.h"
#include <qdict.h>
#include <kmainwindow.h>
#include "thumbnailview.h"

class MainView :public KMainWindow
{
    Q_OBJECT

public:
    MainView( QWidget* parent,  const char* name = 0 );

protected slots:
    bool slotExit();
    void slotOptions();
    void slotConfigureAllImages();
    void slotConfigureImagesOneAtATime();
    void slotSave();
    void slotDeleteSelected();
    void slotSearch();
    void load();
    void slotViewSelected();
    void slotChanges();
    void slotShowAllThumbNails();
    void slotLimitToSelected();

protected:
    void configureImages( bool oneAtATime );
    void loadExtraFiles( const QDict<void>& loadedFiles, const QString& indexDirectory, QString directory );
    void load( const QString& indexDirectory,  const QString& filename, QDomElement elm );
    ImageInfoList selected();
    void wellcome();
    virtual void closeEvent( QCloseEvent* e );
    void setupMenuBar();

private:
    ThumbNailView* _thumbNailView;
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    ImageInfoList _images;
    bool _dirty;
};


#endif /* MAINVIEW_H */

