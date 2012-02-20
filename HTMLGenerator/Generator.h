/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef HTMLGENERATOR_GENERATOR_H
#define HTMLGENERATOR_GENERATOR_H

#include <DB/CategoryPtr.h>
#include <qstring.h>
#include <ImageManager/ImageClient.h>
#include <QProgressDialog>
#include "Utilities/UniqFilenameMapper.h"
#include "Setup.h"
#include <QEventLoop>
#include <KTempDir>
#include "Utilities/Set.h"
#include <QPointer>

namespace DB { class Id; }

namespace HTMLGenerator
{
using Utilities::StringSet;

class Generator :public QProgressDialog, private ImageManager::ImageClient
{
    Q_OBJECT

public:
    Generator( const Setup& setup,  QWidget* parent );
    ~Generator();
    void generate();

protected slots:
    void slotCancelGenerate();
    void showBrowser();

protected:
    bool generateIndexPage( int width, int height );
    bool generateContentPage( int width, int height,
                              const DB::Id& prevInfo, const DB::Id& current, const DB::Id& nextInfo );
    bool linkIndexFile();
    QString populateDescription( QList<DB::CategoryPtr> categories, const DB::ImageInfoPtr info );

public:
    QString namePage( int width, int height, const QString& fileName );
    QString nameImage( const QString& fileName, int size );

    QString createImage( const DB::Id& id, int size );
    QString createVideo( const QString& fileName );

    QString kimFileName( bool relative );
    bool writeToFile( const QString& fileName, const QString& str );
    QString translateToHTML( const QString& );
    int calculateSteps();
    void getThemeInfo( QString* baseDir, QString* name, QString* author );


    virtual void pixmapLoaded( const QString& fileName, const QSize& size,
                               const QSize& fullSize, int angle, const QImage&,
                               const bool loadedOK);
    int maxImageSize();
    void minImageSize( int& width, int& height);

private:
    Setup _setup;
    int _waitCounter;
    int _total;
    KTempDir _tempDir;
    Utilities::UniqFilenameMapper _filenameMapper;
    QSet< QPair<QString,int> > _generatedFiles;
    StringSet _copiedVideos;
    bool _hasEnteredLoop;
    QPointer<QEventLoop> _eventLoop;
};

}



#endif /* HTMLGENERATOR_GENERATOR_H */

