#ifndef MAINVIEW_H
#define MAINVIEW_H
class OptionsDialog;
class ImageConfig;
class QWidgetStack;
class ImageCounter;
class QTimer;
#include "imageinfo.h"
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
    void slotExportToHTML();
    void slotDeleteOption( const QString& optionGroup, const QString& which );
    void slotRenameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue );
    void slotAutoSave();

protected:
    void configureImages( bool oneAtATime );
    void loadExtraFiles( const QDict<void>& loadedFiles, QString directory );
    void load( const QString& filename, QDomElement elm );
    ImageInfoList selected();
    void wellcome();
    virtual void closeEvent( QCloseEvent* e );
    void setupMenuBar();
    void startAutoSaveTimer();
    void save( const QString& fileName );
    void checkForBackupFile();

private:
    ThumbNailView* _thumbNailView;
    OptionsDialog* _optionsDialog;
    ImageConfig* _imageConfigure;
    ImageInfoList _images;
    bool _dirty;
    QWidgetStack* _stack;
    QWidget* _welcome;
    ImageCounter* _counter;
    QTimer* _autoSaveTimer;
};


#endif /* MAINVIEW_H */

