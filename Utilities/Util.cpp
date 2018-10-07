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

#include "Util.h"
#include "Logging.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <Exif/Info.h>
#include <ImageManager/ImageDecoder.h>
#include <ImageManager/RawImageDecoder.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>

#include <KCodecs>
#include <KJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

#include <KIO/DeleteJob>

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImageReader>
#include <QList>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegExp>
#include <QStandardPaths>
#include <QTextCodec>
#include <QUrl>
#include <QVector>

extern "C" {
#include <limits.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
}

namespace {
// Determined experimentally to yield best results (on Seagate 2TB 2.5" disk,
// 5400 RPM).  Performance is very similar at 524288.  Above that, performance
// was significantly worse.  Below that, performance also deteriorated.
// This assumes use of one image scout thread (see DB/ImageScout.cpp).  Without
// a scout thread, performance was about 10-15% worse.
constexpr int MD5_BUFFER_SIZE = 262144;
}

/**
 * Add a line label + info text to the result text if info is not empty.
 * If the result already contains something, a HTML newline is added first.
 * To be used in createInfoText().
 */
static void AddNonEmptyInfo(const QString &label, const QString &info,
                            QString *result) {
    if (info.isEmpty())
        return;
    if (!result->isEmpty())
        *result += QString::fromLatin1("<br/>");
    result->append(label).append(info);
}

/**
 * Given an ImageInfoPtr this function will create an HTML blob about the
 * image. The blob is used in the viewer and in the tool tip box from the
 * thumbnail view.
 *
 * As the HTML text is created, the parameter linkMap is filled with
 * information about hyperlinks. The map maps from an index to a pair of
 * (categoryName, categoryItem). This linkMap is used when the user selects
 * one of the hyberlinks.
 */
QString Utilities::createInfoText( DB::ImageInfoPtr info, QMap< int,QPair<QString,QString> >* linkMap )
{
    Q_ASSERT( info );

    QString result;
    if ( Settings::SettingsData::instance()->showFilename() ) {
        AddNonEmptyInfo(i18n("<b>File Name: </b> "), info->fileName().relative(), &result);
    }

    if ( Settings::SettingsData::instance()->showDate() )  {
        AddNonEmptyInfo(i18n("<b>Date: </b> "), info->date().toString( Settings::SettingsData::instance()->showTime() ? true : false ),
                        &result);
    }

    /* XXX */
    if ( Settings::SettingsData::instance()->showImageSize() && info->mediaType() == DB::Image)  {
        const QSize imageSize = info->size();
        // Do not add -1 x -1 text
        if (imageSize.width() >= 0 && imageSize.height() >= 0) {
            const double megapix = imageSize.width() * imageSize.height() / 1000000.0;
            QString info = i18nc("width x height","%1x%2"
                ,QString::number(imageSize.width())
                ,QString::number(imageSize.height()));
            if (megapix > 0.05) {
                info += i18nc("short for: x megapixels"," (%1MP)"
                    ,QString::number(megapix, 'f', 1));
            }
            const double aspect = (double) imageSize.width() / (double) imageSize.height();
            if (aspect > 1)
                info += i18nc("aspect ratio"," (%1:1)"
                              ,QLocale::system().toString(aspect, 'f', 2));
            else if (aspect >= 0.995 && aspect < 1.005)
                info += i18nc("aspect ratio"," (1:1)");
            else
                info += i18nc("aspect ratio"," (1:%1)"
                              ,QLocale::system().toString(1.0/aspect, 'f', 2));
            AddNonEmptyInfo(i18n("<b>Image Size: </b> "), info, &result);
        }
    }

    if ( Settings::SettingsData::instance()->showRating() ) {
        if ( info->rating() != -1 ) {
            if ( ! result.isEmpty() )
                result += QString::fromLatin1("<br/>");
            QUrl rating;
            rating.setScheme(QString::fromLatin1("kratingwidget"));
            // we don't use the host part, but if we don't set it, we can't use port:
            rating.setHost(QString::fromLatin1("int"));
            rating.setPort(qMin( qMax( static_cast<short int>(0), info->rating() ), static_cast<short int>(10)));
            result += QString::fromLatin1("<img src=\"%1\"/>").arg( rating.toString(QUrl::None) );
        }
    }

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    int link = 0;
    Q_FOREACH( const DB::CategoryPtr category, categories ) {
        const QString categoryName = category->name();
        if ( category->doShow() ) {
            StringSet items = info->itemsOfCategory( categoryName );

            if (Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured()
                    && ! Settings::SettingsData::instance()->untaggedImagesTagVisible()) {

                if (categoryName == Settings::SettingsData::instance()->untaggedCategory()) {
                    if (items.contains(Settings::SettingsData::instance()->untaggedTag())) {
                        items.remove(Settings::SettingsData::instance()->untaggedTag());
                    }
                }
            }

            if (!items.empty()) {
                QString title = QString::fromUtf8("<b>%1: </b> ").arg(category->name());
                QString infoText;
                bool first = true;
                Q_FOREACH( const QString &item, items) {
                    if ( first )
                        first = false;
                    else
                        infoText += QString::fromLatin1( ", " );

                    if ( linkMap ) {
                        ++link;
                        (*linkMap)[link] = QPair<QString,QString>( categoryName, item );
                        infoText += QString::fromLatin1( "<a href=\"%1\">%2</a>").arg( link ).arg( item );
                        infoText += formatAge(category, item, info);
                    }
                    else
                        infoText += item;
                }
                AddNonEmptyInfo(title, infoText, &result);
            }
        }
    }

    if ( Settings::SettingsData::instance()->showLabel()) {
        AddNonEmptyInfo(i18n("<b>Label: </b> "), info->label(), &result);
    }

    if ( Settings::SettingsData::instance()->showDescription() && !info->description().trimmed().isEmpty() )  {
        AddNonEmptyInfo(i18n("<b>Description: </b> "), info->description(),
                        &result);
    }

    QString exifText;
    if ( Settings::SettingsData::instance()->showEXIF() ) {
        typedef QMap<QString,QStringList> ExifMap;
        typedef ExifMap::const_iterator ExifMapIterator;
        ExifMap exifMap = Exif::Info::instance()->infoForViewer( info->fileName(), Settings::SettingsData::instance()->iptcCharset() );

        for( ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt ) {
            if ( exifIt.key().startsWith( QString::fromLatin1( "Exif." ) ) )
                for ( QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt ) {
                    QString exifName = exifIt.key().split( QChar::fromLatin1('.') ).last();
                    AddNonEmptyInfo(QString::fromLatin1( "<b>%1: </b> ").arg(exifName),
                                    *valuesIt, &exifText);
                }
        }

        QString iptcText;
        for( ExifMapIterator exifIt = exifMap.constBegin(); exifIt != exifMap.constEnd(); ++exifIt ) {
            if ( !exifIt.key().startsWith( QString::fromLatin1( "Exif." ) ) )
                for ( QStringList::const_iterator valuesIt = exifIt.value().constBegin(); valuesIt != exifIt.value().constEnd(); ++valuesIt ) {
                    QString iptcName = exifIt.key().split( QChar::fromLatin1('.') ).last();
                    AddNonEmptyInfo(QString::fromLatin1( "<b>%1: </b> ").arg(iptcName),
                                    *valuesIt, &iptcText);
                }
        }

        if ( !iptcText.isEmpty() ) {
            if ( exifText.isEmpty() )
                exifText = iptcText;
            else
                exifText += QString::fromLatin1( "<hr>" ) + iptcText;
        }
    }

    if ( !result.isEmpty() && !exifText.isEmpty() )
        result += QString::fromLatin1( "<hr>" );
    result += exifText;

    return result;
}

using DateSpec = QPair<int, char>;
DateSpec dateDiff(const QDate& birthDate, const QDate& imageDate)
{
    const int bday = birthDate.day();
    const int iday = imageDate.day();
    const int bmonth = birthDate.month();
    const int imonth = imageDate.month();
    const int byear = birthDate.year();
    const int iyear = imageDate.year();

    // Image before birth
    const int diff = birthDate.daysTo(imageDate);
    if (diff < 0)
        return qMakePair(0, 'I');

    if (diff < 31)
        return qMakePair(diff, 'D');

    int months = (iyear-byear)*12;
    months += (imonth-bmonth);
    months += (iday >= bday) ? 0 : -1;

    if ( months < 24)
        return qMakePair(months, 'M');
    else
        return qMakePair(months/12, 'Y');
}

QString formatDate(const DateSpec& date)
{
    if (date.second == 'I')
        return {};
    else if (date.second == 'D')
        return i18np("1 day", "%1 days", date.first);
    else if (date.second == 'M')
        return i18np("1 month", "%1 months", date.first);
    else
        return i18np("1 year", "%1 years", date.first);
}

void test() {
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1971,7,11))) == QString::fromLatin1("0 days"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1971,8,10))) == QString::fromLatin1("30 days"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1971,8,11))) == QString::fromLatin1("1 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1971,8,12))) == QString::fromLatin1("1 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1971,9,10))) == QString::fromLatin1("1 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1971,9,11))) == QString::fromLatin1("2 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,6,10))) == QString::fromLatin1("10 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,6,11))) == QString::fromLatin1("11 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,6,12))) == QString::fromLatin1("11 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,7,10))) == QString::fromLatin1("11 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,7,11))) == QString::fromLatin1("12 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,7,12))) == QString::fromLatin1("12 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1972,12,11))) == QString::fromLatin1("17 month"));
    Q_ASSERT(formatDate(dateDiff(QDate(1971,7,11), QDate(1973,7,11))) == QString::fromLatin1("2 years"));
}

QString Utilities::formatAge(DB::CategoryPtr category, const QString &item, DB::ImageInfoPtr info)
{
    // test(); // I wish I could get my act together to set up a test suite.
    const QDate birthDate = category->birthDate(item);
    const QDate start = info->date().start().date();
    const QDate end = info->date().end().date();

    if (birthDate.isNull() || start.isNull())
        return {};

    if ( start == end)
        return QString::fromUtf8(" (%1)").arg(formatDate(dateDiff(birthDate, start)));
    else {
        DateSpec lower = dateDiff(birthDate,start);
        DateSpec upper = dateDiff(birthDate,end);
        if (lower == upper)
            return QString::fromUtf8(" (%1)").arg(formatDate(lower));
        else if (lower.second == 'I')
            return QString::fromUtf8(" (&lt; %1)").arg(formatDate(upper));
        else {
            if (lower.second == upper.second)
                return QString::fromUtf8(" (%1-%2)").arg(lower.first).arg(formatDate(upper));
            else
                return QString::fromUtf8(" (%1-%2)").arg(formatDate(lower)).arg(formatDate(upper));
        }
    }
}

void Utilities::checkForBackupFile( const QString& fileName, const QString& message )
{
    QString backupName = QFileInfo( fileName ).absolutePath() + QString::fromLatin1("/.#") + QFileInfo( fileName ).fileName();
    QFileInfo backUpFile( backupName);
    QFileInfo indexFile( fileName );

    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() || backUpFile.size() == 0 )
        if ( !( backUpFile.exists() && !message.isNull() ) )
            return;

    int code;
    if ( message.isNull() )
        code = KMessageBox::questionYesNo( nullptr, i18n("Autosave file '%1' exists (size %3 KB) and is newer than '%2'. "
                "Should the autosave file be used?", backupName, fileName, backUpFile.size() >> 10 ),
                i18n("Found Autosave File") );
    else if ( backUpFile.size() > 0 )
        code = KMessageBox::warningYesNo( nullptr,i18n( "<p>Error: Cannot use current database file '%1':</p><p>%2</p>"
                "<p>Do you want to use autosave (%3 - size %4 KB) instead of exiting?</p>"
                "<p><small>(Manually verifying and copying the file might be a good idea)</small></p>", fileName, message, backupName, backUpFile.size() >> 10 ),
                i18n("Recover from Autosave?") );
    else {
        KMessageBox::error( nullptr, i18n( "<p>Error: %1</p><p>Also autosave file is empty, check manually "
                        "if numbered backup files exist and can be used to restore index.xml.</p>", message ) );
        exit(-1);
    }

    if ( code == KMessageBox::Yes ) {
        QFile in( backupName );
        if ( in.open( QIODevice::ReadOnly ) ) {
            QFile out( fileName );
            if (out.open( QIODevice::WriteOnly ) ) {
                char data[1024];
                int len;
                while ( (len = in.read( data, 1024 ) ) )
                    out.write( data, len );
            }
        }
    } else if ( !message.isNull() )
        exit(-1);
}

bool Utilities::ctrlKeyDown()
{
    return QApplication::keyboardModifiers() & Qt::ControlModifier;
}

void Utilities::copyList( const QStringList& from, const QString& directoryTo )
{
    for( QStringList::ConstIterator it = from.constBegin(); it != from.constEnd(); ++it ) {
        QString destFile = directoryTo + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            const bool ok = copy( *it, destFile );
            if ( !ok ) {
                KMessageBox::error( nullptr, i18n("Unable to copy '%1' to '%2'.", *it , destFile ), i18n("Error Running Demo") );
                exit(-1);
            }
        }
    }
}

QString Utilities::setupDemo()
{
    QString demoDir = QString::fromLatin1( "%1/kphotoalbum-demo-%2" ).arg(QDir::tempPath()).arg(QString::fromLocal8Bit( qgetenv( "LOGNAME" ) ));
    QFileInfo fi(demoDir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( demoDir );
        if ( !ok ) {
            KMessageBox::error( nullptr, i18n("Unable to create directory '%1' needed for demo.", demoDir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    // index.xml
    QString demoDB = locateDataFile(QString::fromLatin1("demo/index.xml"));
    if ( demoDB.isEmpty() )
    {
        qCDebug(UtilitiesLog) << "No demo database in standard locations:" << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        exit(-1);
    }
    QString configFile = demoDir + QString::fromLatin1( "/index.xml" );
    copy(demoDB, configFile);

    // Images
    const QStringList kpaDemoDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                QString::fromLatin1("demo"),
                QStandardPaths::LocateDirectory);
    QStringList images;
    Q_FOREACH(const QString &dir, kpaDemoDirs)
    {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.jpg") << QStringLiteral("*.avi"));
        while (it.hasNext()) {
            images.append(it.next());
        }
    }
    copyList( images, demoDir );

    // CategoryImages
    QString catDir = demoDir + QString::fromLatin1("/CategoryImages");
    fi = QFileInfo(catDir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( catDir  );
        if ( !ok ) {
            KMessageBox::error( nullptr, i18n("Unable to create directory '%1' needed for demo.", catDir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    const QStringList kpaDemoCatDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                QString::fromLatin1("demo/CategoryImages"),
                QStandardPaths::LocateDirectory);
    QStringList catImages;
    Q_FOREACH(const QString &dir, kpaDemoCatDirs)
    {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.jpg"));
        while (it.hasNext()) {
            catImages.append(it.next());
        }
    }
    copyList( catImages, catDir );

    return configFile;
}

bool Utilities::copy( const QString& from, const QString& to )
{
    if ( QFileInfo(to).exists())
        QDir().remove(to);
    return QFile::copy(from,to);
}

bool Utilities::makeHardLink( const QString& from, const QString& to )
{
    if (link(from.toLocal8Bit().constData(), to.toLocal8Bit().constData()) != 0)
        return false;
    else
        return true;
}

bool Utilities::makeSymbolicLink( const QString& from, const QString& to )
{
    if (symlink(from.toLocal8Bit().constData(), to.toLocal8Bit().constData()) != 0)
        return false;
    else
        return true;
}

bool Utilities::canReadImage( const DB::FileName& fileName )
{
    bool fastMode = !Settings::SettingsData::instance()->ignoreFileExtension();
    QMimeDatabase::MatchMode mode = fastMode ? QMimeDatabase::MatchExtension : QMimeDatabase::MatchDefault;
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile( fileName.absolute(), mode );

    return QImageReader::supportedMimeTypes().contains( mimeType.name().toUtf8() )
            || ImageManager::ImageDecoder::mightDecode( fileName );
}


QString Utilities::locateDataFile(const QString& fileName)
{
    return QStandardPaths::locate(QStandardPaths::DataLocation, fileName);
}

QString Utilities::readFile( const QString& fileName )
{
    if ( fileName.isEmpty() ) {
        KMessageBox::error( nullptr, i18n("<p>No file name given!</p>") );
        return QString();
    }

    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        //KMessageBox::error( nullptr, i18n("Could not open file %1").arg( fileName ) );
        return QString();
    }

    QTextStream stream( &file );
    QString content = stream.readAll();
    file.close();

    return content;
}

namespace Utilities
{
QString normalizedFileName( const QString& fileName )
{
    return QFileInfo(fileName).absoluteFilePath();
}

QString dereferenceSymLinks( const QString& fileName )
{
    QFileInfo fi(fileName);
    int rounds = 256;
    while (fi.isSymLink() && --rounds > 0)
        fi = QFileInfo(fi.readLink());
    if (rounds == 0)
        return QString();
    return fi.filePath();
}
}

QString Utilities::stripEndingForwardSlash( const QString& fileName )
{
    static QString slash = QString::fromLatin1("/");
    if ( fileName.endsWith( slash ) )
        return fileName.left( fileName.length()-1);
    else
        return fileName;
}

QString Utilities::relativeFolderName( const QString& fileName)
{
    int index= fileName.lastIndexOf( QChar::fromLatin1('/'), -1);
    if (index == -1)
        return QString();
    else
        return fileName.left( index );
}

void Utilities::deleteDemo()
{
    QString dir = QString::fromLatin1( "%1/kphotoalbum-demo-%2" ).arg(QDir::tempPath()).arg(QString::fromLocal8Bit( qgetenv( "LOGNAME" ) ) );
    QUrl demoUrl = QUrl::fromLocalFile( dir );
    KJob *delDemoJob = KIO::del( demoUrl );
    KJobWidgets::setWindow( delDemoJob, MainWindow::Window::theMainWindow());
    delDemoJob->exec();
}

QString Utilities::absoluteImageFileName( const QString& relativeName )
{
    return stripEndingForwardSlash( Settings::SettingsData::instance()->imageDirectory() ) + QString::fromLatin1( "/" ) + relativeName;
}

QString Utilities::imageFileNameToAbsolute( const QString& fileName )
{
    if ( fileName.startsWith( Settings::SettingsData::instance()->imageDirectory() ) )
        return fileName;
    else if ( fileName.startsWith( QString::fromLatin1("file://") ) )
        return imageFileNameToAbsolute( fileName.mid( 7 ) ); // 7 == length("file://")
    else if ( fileName.startsWith( QString::fromLatin1("/") ) )
        return QString(); // Not within our image root
    else
        return absoluteImageFileName( fileName );
}

bool operator>( const QPoint& p1, const QPoint& p2)
{
    return p1.y() > p2.y() || (p1.y() == p2.y() && p1.x() > p2.x() );
}

bool operator<( const QPoint& p1, const QPoint& p2)
{
    return p1.y() < p2.y() || ( p1.y() == p2.y() && p1.x() < p2.x() );
}

const QSet<QString>& Utilities::supportedVideoExtensions()
{
    static QSet<QString> videoExtensions;
    if ( videoExtensions.empty() ) {
        videoExtensions.insert( QString::fromLatin1( "3gp" ) );
        videoExtensions.insert( QString::fromLatin1( "avi" ) );
        videoExtensions.insert( QString::fromLatin1( "mp4" ) );
        videoExtensions.insert( QString::fromLatin1( "m4v" ) );
        videoExtensions.insert( QString::fromLatin1( "mpeg" ) );
        videoExtensions.insert( QString::fromLatin1( "mpg" ) );
        videoExtensions.insert( QString::fromLatin1( "qt" ) );
        videoExtensions.insert( QString::fromLatin1( "mov" ) );
        videoExtensions.insert( QString::fromLatin1( "moov" ) );
        videoExtensions.insert( QString::fromLatin1( "qtvr" ) );
        videoExtensions.insert( QString::fromLatin1( "rv" ) );
        videoExtensions.insert( QString::fromLatin1( "3g2" ) );
        videoExtensions.insert( QString::fromLatin1( "fli" ) );
        videoExtensions.insert( QString::fromLatin1( "flc" ) );
        videoExtensions.insert( QString::fromLatin1( "mkv" ) );
        videoExtensions.insert( QString::fromLatin1( "mng" ) );
        videoExtensions.insert( QString::fromLatin1( "asf" ) );
        videoExtensions.insert( QString::fromLatin1( "asx" ) );
        videoExtensions.insert( QString::fromLatin1( "wmp" ) );
        videoExtensions.insert( QString::fromLatin1( "wmv" ) );
        videoExtensions.insert( QString::fromLatin1( "ogm" ) );
        videoExtensions.insert( QString::fromLatin1( "rm" ) );
        videoExtensions.insert( QString::fromLatin1( "flv" ) );
        videoExtensions.insert( QString::fromLatin1( "webm" ) );
        videoExtensions.insert( QString::fromLatin1( "mts" ) );
        videoExtensions.insert( QString::fromLatin1( "ogg" ) );
        videoExtensions.insert( QString::fromLatin1( "ogv" ) );
        videoExtensions.insert( QString::fromLatin1( "m2ts" ) );
    }
    return videoExtensions;
}
bool Utilities::isVideo( const DB::FileName& fileName )
{
    QFileInfo fi( fileName.relative() );
    QString ext = fi.suffix().toLower();
    return supportedVideoExtensions().contains( ext );
}

bool Utilities::isRAW( const DB::FileName& fileName )
{
    return ImageManager::RAWImageDecoder::isRAW( fileName );
}


QImage Utilities::scaleImage(const QImage &image, int w, int h, Qt::AspectRatioMode mode )
{
    return image.scaled( w, h, mode, Settings::SettingsData::instance()->smoothScale() ? Qt::SmoothTransformation : Qt::FastTransformation );
}

QImage Utilities::scaleImage(const QImage &image, const QSize& s, Qt::AspectRatioMode mode )
{
    return scaleImage( image, s.width(), s.height(), mode );
}

QString Utilities::cStringWithEncoding( const char *c_str, const QString& charset )
{
    QTextCodec* codec = QTextCodec::codecForName( charset.toLatin1() );
    if (!codec)
        codec = QTextCodec::codecForLocale();
    return codec->toUnicode( c_str );
}

DB::MD5 Utilities::MD5Sum( const DB::FileName& fileName )
{
    DB::MD5 checksum;
    QFile file( fileName.absolute() );
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QCryptographicHash md5calculator(QCryptographicHash::Md5);
        while ( !file.atEnd() ) {
            QByteArray md5Buffer( file.read( MD5_BUFFER_SIZE ) );
            md5calculator.addData( md5Buffer );
        }
        file.close();
        checksum = DB::MD5(QString::fromLatin1(md5calculator.result().toHex()));
    }
    return checksum;
}

QColor Utilities::contrastColor( const QColor& col )
{
    if ( col.red() < 127 && col.green() < 127 && col.blue() < 127 )
        return Qt::white;
    else
        return Qt::black;
}

void Utilities::saveImage( const DB::FileName& fileName, const QImage& image, const char* format )
{
    const QFileInfo info(fileName.absolute());
    QDir().mkpath(info.path());
     const bool ok = image.save(fileName.absolute(),format);
    Q_ASSERT(ok); Q_UNUSED(ok);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
