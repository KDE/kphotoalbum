// SPDX-FileCopyrightText: 2003-2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005 Steffen Hansen <hansen@kde.org>
// SPDX-FileCopyrightText: 2006-2009 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007-2012 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2010 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012 Rex Dieter <rdieter@math.unl.edu>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2016 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2018 Robert Krawitz <rlk@alum.mit.edu>

// SPDX-License-Identifier: GPL-2.0-or-later

#include "FileExtensions.h"

#include <kpabase/SettingsData.h>
#include <kpabase/config-kpa-kdcraw.h>
#ifdef HAVE_KDCRAW
#include <KDCRAW/RawFiles>
#include <libkdcraw_version.h>
#endif

#include <QFileInfo>
#include <QSet>
#include <QString>

namespace
{
void _initializeExtensionLists(QStringList &rawExtensions, QStringList &standardExtensions, QStringList &ignoredExtensions)
{
    static QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    static bool extensionListsInitialized = false;
    if (!extensionListsInitialized) {
#ifdef HAVE_KDCRAW
        _rawExtensions = QString::fromLatin1(raw_file_extentions).split(QChar::fromLatin1(' '), Qt::SkipEmptyParts);
#endif /* HAVE_KDCRAW */
        for (QStringList::iterator it = _rawExtensions.begin(); it != _rawExtensions.end(); ++it)
            (*it).remove(QString::fromUtf8("*."));

        _standardExtensions << QString::fromLatin1("jpg")
                            << QString::fromLatin1("JPG")
                            << QString::fromLatin1("jpeg")
                            << QString::fromLatin1("JPEG")
                            << QString::fromLatin1("tif")
                            << QString::fromLatin1("TIF")
                            << QString::fromLatin1("tiff")
                            << QString::fromLatin1("TIFF")
                            << QString::fromLatin1("png")
                            << QString::fromLatin1("PNG");
        _ignoredExtensions << QString::fromLatin1("thm") // Thumbnails
                           << QString::fromLatin1("THM")
                           << QString::fromLatin1("thumb") // thumbnail files
                           // from dcraw
                           << QString::fromLatin1("ctg") // Catalog files
                           << QString::fromLatin1("gz") // Compressed files
                           << QString::fromLatin1("Z")
                           << QString::fromLatin1("bz2")
                           << QString::fromLatin1("zip")
                           << QString::fromLatin1("xml")
                           << QString::fromLatin1("XML")
                           << QString::fromLatin1("html")
                           << QString::fromLatin1("HTML")
                           << QString::fromLatin1("htm")
                           << QString::fromLatin1("HTM")
                           << QString::fromLatin1("pp3") // RawTherapee Sidecar files
                           << QString::fromLatin1("PP3")
                           << QString::fromLatin1("xmp") // Other sidecars
                           << QString::fromLatin1("XMP")
                           << QString::fromLatin1("pto") // Hugin sidecars
                           << QString::fromLatin1("PTO");

        QChar dot(QChar::fromLatin1('.'));
        for (QStringList::iterator it = _rawExtensions.begin(); it != _rawExtensions.end(); ++it)
            if (!(*it).startsWith(dot))
                *it = dot + *it;
        for (QStringList::iterator it = _standardExtensions.begin(); it != _standardExtensions.end(); ++it)
            if (!(*it).startsWith(dot))
                *it = dot + *it;
        for (QStringList::iterator it = _ignoredExtensions.begin(); it != _ignoredExtensions.end(); ++it)
            if (!(*it).startsWith(dot))
                *it = dot + *it;

        extensionListsInitialized = true;
    }

    rawExtensions = _rawExtensions;
    standardExtensions = _standardExtensions;
    ignoredExtensions = _ignoredExtensions;
}
bool _fileExistsWithExtensions(const DB::FileName &fileName,
                               const QStringList &extensionList)
{
    QString baseFileName = fileName.absolute();
    int extStart = baseFileName.lastIndexOf(QChar::fromLatin1('.'));
    // We're interested in xxx.yyy, not .yyy
    if (extStart <= 1)
        return false;
    baseFileName.remove(extStart, baseFileName.length() - extStart);
    for (QStringList::ConstIterator it = extensionList.begin();
         it != extensionList.end(); ++it) {
        if (QFile::exists(baseFileName + *it))
            return true;
    }
    return false;
}

bool _fileIsKnownWithExtensions(const DB::FileNameSet &files,
                                const DB::FileName &fileName,
                                const QStringList &extensionList)
{
    QString baseFileName = fileName.absolute();
    int extStart = baseFileName.lastIndexOf(QChar::fromLatin1('.'));
    if (extStart <= 1)
        return false;
    baseFileName.remove(extStart, baseFileName.length() - extStart);
    for (QStringList::ConstIterator it = extensionList.begin();
         it != extensionList.end(); ++it) {
        if (files.contains(DB::FileName::fromAbsolutePath(baseFileName + *it)))
            return true;
    }
    return false;
}

bool _fileEndsWithExtensions(const DB::FileName &fileName,
                             const QStringList &extensionList)
{
    for (QStringList::ConstIterator it = extensionList.begin();
         it != extensionList.end(); ++it) {
        if (fileName.relative().endsWith(*it, Qt::CaseInsensitive))
            return true;
    }
    return false;
}
} // namespace

bool KPABase::fileCanBeSkipped(const DB::FileNameSet &loadedFiles, const DB::FileName &imageFile)
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists(_rawExtensions, _standardExtensions, _ignoredExtensions);

    // We're not interested in thumbnail and other files.
    if (_fileEndsWithExtensions(imageFile, _ignoredExtensions))
        return true;

    // If we *are* interested in raw files even when other equivalent
    // non-raw files are available, then we're interested in this file.
    if (!(Settings::SettingsData::instance()->skipRawIfOtherMatches()))
        return false;

    // If the file ends with something other than a known raw extension,
    // we're interested in it.
    if (!_fileEndsWithExtensions(imageFile, _rawExtensions))
        return false;

    // At this point, the file ends with a known raw extension, and we're
    // not interested in raw files when other non-raw files are available.
    // So search for an existing file with one of the standard
    // extensions.
    //
    // This may not be the best way to do this, but it's using the
    // same algorithm as _mightDecode above.
    // -- Robert Krawitz rlk@alum.mit.edu 2007-07-22

    return _fileIsKnownWithExtensions(loadedFiles, imageFile, _standardExtensions);
}

bool KPABase::isUsableRawImage(const DB::FileName &imageFile, FileTypePreference preference)
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists(_rawExtensions, _standardExtensions, _ignoredExtensions);

    if (preference == FileTypePreference::PreferNonRawFile && _fileExistsWithExtensions(imageFile, _standardExtensions))
        return false;
    if (_fileEndsWithExtensions(imageFile, _rawExtensions))
        return true;
    return false;
}

QStringList KPABase::rawExtensions()
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists(_rawExtensions, _standardExtensions, _ignoredExtensions);
    return _rawExtensions;
}

bool KPABase::isVideo(const DB::FileName &fileName)
{
    QFileInfo fi(fileName.relative());
    QString ext = fi.suffix().toLower();
    return videoExtensions().contains(ext);
}

const QSet<QString> &KPABase::videoExtensions()
{
    static QSet<QString> videoExtensions;
    if (videoExtensions.empty()) {
        videoExtensions.insert(QString::fromLatin1("3gp"));
        videoExtensions.insert(QString::fromLatin1("avi"));
        videoExtensions.insert(QString::fromLatin1("mp4"));
        videoExtensions.insert(QString::fromLatin1("m4v"));
        videoExtensions.insert(QString::fromLatin1("mpeg"));
        videoExtensions.insert(QString::fromLatin1("mpg"));
        videoExtensions.insert(QString::fromLatin1("qt"));
        videoExtensions.insert(QString::fromLatin1("mov"));
        videoExtensions.insert(QString::fromLatin1("moov"));
        videoExtensions.insert(QString::fromLatin1("qtvr"));
        videoExtensions.insert(QString::fromLatin1("rv"));
        videoExtensions.insert(QString::fromLatin1("3g2"));
        videoExtensions.insert(QString::fromLatin1("fli"));
        videoExtensions.insert(QString::fromLatin1("flc"));
        videoExtensions.insert(QString::fromLatin1("mkv"));
        videoExtensions.insert(QString::fromLatin1("mng"));
        videoExtensions.insert(QString::fromLatin1("asf"));
        videoExtensions.insert(QString::fromLatin1("asx"));
        videoExtensions.insert(QString::fromLatin1("wmp"));
        videoExtensions.insert(QString::fromLatin1("wmv"));
        videoExtensions.insert(QString::fromLatin1("ogm"));
        videoExtensions.insert(QString::fromLatin1("rm"));
        videoExtensions.insert(QString::fromLatin1("flv"));
        videoExtensions.insert(QString::fromLatin1("webm"));
        videoExtensions.insert(QString::fromLatin1("mts"));
        videoExtensions.insert(QString::fromLatin1("ogg"));
        videoExtensions.insert(QString::fromLatin1("ogv"));
        videoExtensions.insert(QString::fromLatin1("m2ts"));
    }
    return videoExtensions;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
