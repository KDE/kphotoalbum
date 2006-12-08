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
#include "Setup.h"
#include "ImageSizeCheckBox.h"
void HTMLGenerator::Setup::setTitle( const QString& title )
{
    _title = title;
}

QString HTMLGenerator::Setup::title() const
{
    return _title;
}

void HTMLGenerator::Setup::setBaseDir( const QString& baseDir )
{
    _baseDir = baseDir;
}

QString HTMLGenerator::Setup::baseDir() const
{
    return _baseDir;
}

void HTMLGenerator::Setup::setBaseURL( const QString& baseURL )
{
    _baseURL = baseURL;
}

QString HTMLGenerator::Setup::baseURL() const
{
    return _baseURL;
}

void HTMLGenerator::Setup::setDestURL( const QString& destURL )
{
    _destURL = destURL;
}

QString HTMLGenerator::Setup::destURL() const
{
    return _destURL;
}

void HTMLGenerator::Setup::setOutputDir( const QString& outputDir )
{
    _outputDir = outputDir;
}

QString HTMLGenerator::Setup::outputDir() const
{
    return _outputDir;
}

void HTMLGenerator::Setup::setThumbSize( int thumbSize )
{
    _thumbSize = thumbSize;
}

int HTMLGenerator::Setup::thumbSize() const
{
    return _thumbSize;
}

void HTMLGenerator::Setup::setDescription( const QString& description )
{
    _description = description;
}

QString HTMLGenerator::Setup::description() const
{
    return _description;
}

void HTMLGenerator::Setup::setNumOfCols( const int numOfCols )
{
    _numOfCols = numOfCols;
}

int HTMLGenerator::Setup::numOfCols() const
{
    return _numOfCols;
}

void HTMLGenerator::Setup::setGenerateKimFile( const bool generateKimFile )
{
    _generateKimFile = generateKimFile;
}

bool HTMLGenerator::Setup::generateKimFile() const
{
    return _generateKimFile;
}

void HTMLGenerator::Setup::setThemePath( const QString& theme )
{
    _theme = theme;
}

QString HTMLGenerator::Setup::themePath() const
{
    return _theme;
}

void HTMLGenerator::Setup::setIncludeCategory( const QString& category, bool include )
{
    _includeCategory[category] = include;
}

bool HTMLGenerator::Setup::includeCategory( const QString& category ) const
{
    return _includeCategory[category];
}

void HTMLGenerator::Setup::setResolutions( const QValueList<ImageSizeCheckBox*>& sizes )
{
    _resolutions = sizes;
}

const QValueList<HTMLGenerator::ImageSizeCheckBox*>& HTMLGenerator::Setup::activeResolutions() const
{
    return _resolutions;
}

void HTMLGenerator::Setup::setImageList( const QStringList& files )
{
    _images = files;
}

const QStringList& HTMLGenerator::Setup::imageList() const
{
    return _images;
}

void HTMLGenerator::Setup::setInlineMovies( bool doInline )
{
    _inlineMovies = doInline;
}

bool HTMLGenerator::Setup::inlineMovies() const
{
    return _inlineMovies;

}
