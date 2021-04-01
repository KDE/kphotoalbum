// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Info.h"

#include <kpabase/Logging.h>

#include <kpabase/FileName.h>
#include <kpabase/SettingsData.h>
#include <kpabase/StringSet.h>

#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <exiv2/exv_conf.h>
#include <exiv2/image.hpp>
#include <exiv2/version.hpp>

using namespace Exif;

namespace
{
QString cStringWithEncoding(const char *c_str, const QString &charset)
{
    QTextCodec *codec = QTextCodec::codecForName(charset.toLatin1());
    if (!codec)
        codec = QTextCodec::codecForLocale();
    return codec->toUnicode(c_str);
}

} // namespace

Info *Info::s_instance = nullptr;

QMap<QString, QStringList> Info::info(const DB::FileName &fileName, StringSet wantedKeys, bool returnFullExifName, const QString &charset)
{
    QMap<QString, QStringList> result;

    try {
        Metadata data = metadata(exifInfoFile(fileName));

        for (Exiv2::ExifData::const_iterator i = data.exif.begin(); i != data.exif.end(); ++i) {
            QString key = QString::fromLocal8Bit(i->key().c_str());
            m_keys.insert(key);

            if (wantedKeys.contains(key)) {
                QString text = key;
                if (!returnFullExifName)
                    text = key.split(QLatin1String(".")).last();

                std::ostringstream stream;
                stream << *i;
                QString str(cStringWithEncoding(stream.str().c_str(), charset));
                result[text] += str;
            }
        }

        for (Exiv2::IptcData::const_iterator i = data.iptc.begin(); i != data.iptc.end(); ++i) {
            QString key = QString::fromLatin1(i->key().c_str());
            m_keys.insert(key);

            if (wantedKeys.contains(key)) {
                QString text = key;
                if (!returnFullExifName)
                    text = key.split(QString::fromLatin1(".")).last();

                std::ostringstream stream;
                stream << *i;
                QString str(cStringWithEncoding(stream.str().c_str(), charset));
                result[text] += str;
            }
        }
    } catch (...) {
    }

    return result;
}

Info *Info::instance()
{
    if (!s_instance)
        s_instance = new Info;
    return s_instance;
}

StringSet Info::availableKeys()
{
    return m_keys;
}

QMap<QString, QStringList> Info::infoForViewer(const DB::FileName &fileName, const QString &charset)
{
    return info(fileName, ::Settings::SettingsData::instance()->exifForViewer(), false, charset);
}

QMap<QString, QStringList> Info::infoForDialog(const DB::FileName &fileName, const QString &charset)
{
    auto keys = ::Settings::SettingsData::instance()->exifForDialog();
    if (keys.isEmpty())
        keys = standardKeys();
    return info(fileName, keys, true, charset);
}

StringSet Info::standardKeys()
{
    static StringSet res;

    if (!res.empty())
        return res;

    QList<const Exiv2::TagInfo *> tags;
    std::ostringstream s;

    const Exiv2::GroupInfo *gi = Exiv2::ExifTags::groupList();
    while (gi->tagList_ != nullptr) {
        Exiv2::TagListFct tl = gi->tagList_;
        const Exiv2::TagInfo *ti = tl();

        while (ti->tag_ != 0xFFFF) {
            tags << ti;
            ++ti;
        }
        ++gi;
    }

    for (QList<const Exiv2::TagInfo *>::iterator it = tags.begin(); it != tags.end(); ++it) {
        while ((*it)->tag_ != 0xffff) {
            res.insert(QString::fromLatin1(Exiv2::ExifKey(**it).key().c_str()));
            ++(*it);
        }
    }

    // IPTC tags use yet another format...
    Exiv2::IptcDataSets::dataSetList(s);

    QStringList lines = QString(QLatin1String(s.str().c_str())).split(QChar::fromLatin1('\n'));
    for (QStringList::const_iterator it = lines.constBegin(); it != lines.constEnd(); ++it) {
        if (it->isEmpty())
            continue;
        QStringList fields = it->split(QChar::fromLatin1('\t'));
        if (fields.size() == 7) {
            QString id = fields[4];
            if (id.endsWith(QChar::fromLatin1(',')))
                id.chop(1);
            res.insert(id);
        } else {
            fields = it->split(QLatin1String(", "));
            if (fields.size() >= 11) {
                res.insert(fields[8]);
            } else {
                qCWarning(ExifLog) << "Unparsable output from exiv2 library: " << *it;
                continue;
            }
        }
    }
    return res;
}

Info::Info()
{
    m_keys = standardKeys();
}

void Exif::writeExifInfoToFile(const DB::FileName &srcName, const QString &destName, const QString &imageDescription)
{
    // Load Exif from source image
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(QFile::encodeName(srcName.absolute()).data());
    image->readMetadata();
    Exiv2::ExifData data = image->exifData();

    // Modify Exif information from database.
    data["Exif.Image.ImageDescription"] = imageDescription.toLocal8Bit().data();

    image = Exiv2::ImageFactory::open(QFile::encodeName(destName).data());
    image->setExifData(data);
    image->writeMetadata();
}

/**
 * Some Canon cameras stores Exif info in files ending in .thm, so we need to use those files for fetching Exif info
 * if they exists.
 */
DB::FileName Exif::Info::exifInfoFile(const DB::FileName &fileName)
{
    QString dirName = QFileInfo(fileName.relative()).path();
    QString baseName = QFileInfo(fileName.relative()).baseName();
    DB::FileName name = DB::FileName::fromRelativePath(dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1(".thm"));
    if (name.exists())
        return name;

    name = DB::FileName::fromRelativePath(dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1(".THM"));
    if (name.exists())
        return name;

    return fileName;
}

Exif::Metadata Exif::Info::metadata(const DB::FileName &fileName)
{
    try {
        Exif::Metadata result;
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(QFile::encodeName(fileName.absolute()).data());
        Q_ASSERT(image.get() != nullptr);
        image->readMetadata();
        result.exif = image->exifData();
        result.iptc = image->iptcData();
        result.comment = image->comment();
        return result;
    } catch (...) {
    }
    return Exif::Metadata();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
