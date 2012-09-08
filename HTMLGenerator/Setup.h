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
#ifndef HTMLGENERATOR_SETUP_H
#define HTMLGENERATOR_SETUP_H

#include <qstring.h>
#include <qmap.h>
#include <QList>
#include "DB/FileNameList.h"

namespace HTMLGenerator
{
class ImageSizeCheckBox;

class Setup
{
public:
    Setup();

    void setTitle( const QString& title );
    QString title() const;

    void setBaseDir( const QString& baseDir );
    QString baseDir() const;

    void setBaseURL( const QString& baseURL );
    QString baseURL() const;

    void setDestURL( const QString& destURL );
    QString destURL() const;

    void setOutputDir( const QString& outputDir );
    QString outputDir() const;

    void setThumbSize( int thumbSize );
    int thumbSize() const;

    void setCopyright( const QString& copyright );
    QString copyright() const;

    void setDate( bool date );
    bool date() const;

    void setDescription( const QString& description );
    QString description() const;

    void setNumOfCols( int numOfCols );
    int numOfCols() const;

    void setGenerateKimFile( bool generateKimFile );
    bool generateKimFile() const;

    void setThemePath( const QString& theme );
    QString themePath() const;

    void setIncludeCategory( const QString& category, bool include );
    bool includeCategory( const QString& category ) const;

     void setResolutions( const QList<ImageSizeCheckBox*>& sizes );
     const QList<HTMLGenerator::ImageSizeCheckBox*>& activeResolutions() const;

    void setImageList(const DB::FileNameList& files);
    DB::FileNameList imageList() const;

    void setInlineMovies( bool inlineMovie );
    bool inlineMovies() const;

    void setHtml5Video( bool html5Video );
    bool html5Video() const;

    void setHtml5VideoGenerate( bool html5VideoGenerate );
    bool html5VideoGenerate() const;

private:
    QString _title;
    QString _baseDir;
    QString _baseURL;
    QString _destURL;
    QString _outputDir;
    int _thumbSize;
    QString _copyright;
    bool _date;
    QString _description;
    int _numOfCols;
    bool _generateKimFile;
    QString _theme;
    QMap<QString,bool> _includeCategory;
    QList<ImageSizeCheckBox*> _resolutions;
    DB::FileNameList _images;
    bool _inlineMovies;
    bool _html5Video;
    bool _html5VideoGenerate;
};

}

#endif /* HTMLGENERATOR_SETUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
