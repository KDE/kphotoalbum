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
#include "Setup.h"

#include <QList>

HTMLGenerator::Setup::Setup()
    : m_images()
{
    /* nop */
}

void HTMLGenerator::Setup::setTitle(const QString &title)
{
    m_title = title;
}

QString HTMLGenerator::Setup::title() const
{
    return m_title;
}

void HTMLGenerator::Setup::setBaseDir(const QString &baseDir)
{
    m_baseDir = baseDir;
}

QString HTMLGenerator::Setup::baseDir() const
{
    return m_baseDir;
}

void HTMLGenerator::Setup::setBaseURL(const QString &baseURL)
{
    m_baseURL = baseURL;
}

QString HTMLGenerator::Setup::baseURL() const
{
    return m_baseURL;
}

void HTMLGenerator::Setup::setDestURL(const QString &destURL)
{
    m_destURL = destURL;
}

QString HTMLGenerator::Setup::destURL() const
{
    return m_destURL;
}

void HTMLGenerator::Setup::setOutputDir(const QString &outputDir)
{
    m_outputDir = outputDir;
}

QString HTMLGenerator::Setup::outputDir() const
{
    return m_outputDir;
}

void HTMLGenerator::Setup::setThumbSize(int thumbSize)
{
    m_thumbSize = thumbSize;
}

int HTMLGenerator::Setup::thumbSize() const
{
    return m_thumbSize;
}

void HTMLGenerator::Setup::setCopyright(const QString &copyright)
{
    m_copyright = copyright;
}

QString HTMLGenerator::Setup::copyright() const
{
    return m_copyright;
}

void HTMLGenerator::Setup::setDate(const bool date)
{
    m_date = date;
}

bool HTMLGenerator::Setup::date() const
{
    return m_date;
}

void HTMLGenerator::Setup::setDescription(const QString &description)
{
    m_description = description;
}

QString HTMLGenerator::Setup::description() const
{
    return m_description;
}

void HTMLGenerator::Setup::setNumOfCols(const int numOfCols)
{
    m_numOfCols = numOfCols;
}

int HTMLGenerator::Setup::numOfCols() const
{
    return m_numOfCols;
}

void HTMLGenerator::Setup::setGenerateKimFile(const bool generateKimFile)
{
    m_generateKimFile = generateKimFile;
}

bool HTMLGenerator::Setup::generateKimFile() const
{
    return m_generateKimFile;
}

void HTMLGenerator::Setup::setThemePath(const QString &theme)
{
    m_theme = theme;
}

QString HTMLGenerator::Setup::themePath() const
{
    return m_theme;
}

void HTMLGenerator::Setup::setIncludeCategory(const QString &category, bool include)
{
    m_includeCategory[category] = include;
}

bool HTMLGenerator::Setup::includeCategory(const QString &category) const
{
    return m_includeCategory[category];
}

void HTMLGenerator::Setup::setResolutions(const QList<ImageSizeCheckBox *> &sizes)
{
    m_resolutions = sizes;
}

const QList<HTMLGenerator::ImageSizeCheckBox *> &HTMLGenerator::Setup::activeResolutions() const
{
    return m_resolutions;
}

void HTMLGenerator::Setup::setImageList(const DB::FileNameList &files)
{
    m_images = files;
}

DB::FileNameList HTMLGenerator::Setup::imageList() const
{
    return m_images;
}

void HTMLGenerator::Setup::setInlineMovies(bool doInline)
{
    m_inlineMovies = doInline;
}

bool HTMLGenerator::Setup::inlineMovies() const
{
    return m_inlineMovies;
}

void HTMLGenerator::Setup::setHtml5Video(bool doHtml5Video)
{
    m_html5Video = doHtml5Video;
}

bool HTMLGenerator::Setup::html5Video() const
{
    return m_html5Video;
}

void HTMLGenerator::Setup::setHtml5VideoGenerate(bool doHtml5VideoGenerate)
{
    m_html5VideoGenerate = doHtml5VideoGenerate;
}

bool HTMLGenerator::Setup::html5VideoGenerate() const
{
    return m_html5VideoGenerate;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
