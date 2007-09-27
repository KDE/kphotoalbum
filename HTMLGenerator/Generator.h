/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef HTMLGENERATOR_GENERATOR_H
#define HTMLGENERATOR_GENERATOR_H

#include <qobject.h>
#include <qstring.h>
#include <ImageManager/ImageClient.h>
#include <qprogressdialog.h>
#include "Utilities/Util.h"
#include "Utilities/Set.h"
#include "Setup.h"

namespace HTMLGenerator
{
using Utilities::StringSet;

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
    Utilities::Set< QPair<QString,int> > _generatedFiles;
    StringSet _copiedVideos;
    bool _hasEnteredLoop;
};

};



#endif /* HTMLGENERATOR_GENERATOR_H */

