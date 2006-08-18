#ifndef HTMLGENERATOR_GENERATOR_H
#define HTMLGENERATOR_GENERATOR_H

#include <qobject.h>
#include <qstring.h>
#include <ImageManager/ImageClient.h>
#include <qprogressdialog.h>
#include "Utilities/Util.h"
#include "Setup.h"

namespace HTMLGenerator
{
class Generator :public QProgressDialog, private ImageManager::ImageClient
{
    Q_OBJECT

public:
    Generator( const Setup& setup,  QWidget* parent );
    void generate();

protected slots:
    void slotCancelGenerate();
    void showBrowser();

protected:
    bool generateIndexPage( int width, int height );
    bool generateContentPage( int width, int height, const QString& prevInfo, const QString& info, const QString& nextInfo );
    bool linkIndexFile();

public:
    QString namePage( int width, int height, const QString& fileName );
    QString nameImage( const QString& fileName, int size );

    QString createImage( const QString& fileName, int size );
    QString createVideo( const QString& fileName );

    QString kimFileName( bool relative );
    bool writeToFile( const QString& fileName, const QString& str );
    QString translateToHTML( const QString& );
    int calculateSteps();
    void getThemeInfo( QString* baseDir, QString* name, QString* author );


    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&,
                               bool loadedOK );
    int maxImageSize();

private:
    Setup _setup;
    int _waitCounter;
    int _total;
    QString _tempDir;
    Utilities::UniqNameMap _nameMap;
    Set< QPair<QString,int> > _generatedFiles;
    StringSet _copiedVideos;
    bool _hasEnteredLoop;
};

};



#endif /* HTMLGENERATOR_GENERATOR_H */

