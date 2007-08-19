/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2006-12-09
 * Description : a tread-safe dcraw program interface
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * NOTE: Do not use kdDebug() in this implementation because 
 *       it will be multithreaded. Use qDebug() instead. 
 *       See B.K.O #133026 for details.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

extern "C"
{
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
}

// C++ includes.

#include <cstdlib>
#include <cstdio>
#include <cmath>

// Qt Includes.

#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QByteArray>
#include <QTimer>
#include <QFileInfo>
#include <QApplication>
#include <QMutex>
#include <QWaitCondition>

// KDE includes.

#include <kprocess.h>
#include <kstandarddirs.h>
#include <kshell.h>

// Local includes.

#include "rawfiles.h"
#include "dcrawbinary.h"
#include "kdcraw.h"
#include "kdcraw.moc"

namespace KDcrawIface
{

class KDcrawPriv
{
public:

    KDcrawPriv()
    {
        process    = 0;
        queryTimer = 0;
        data       = 0;
        width      = 0;
        height     = 0;
        rgbmax     = 0;
        dataPos    = 0;
    }

    char           *data;

    int             dataPos;
    int             width;
    int             height;
    int             rgbmax;

    QString         filePath;

    QMutex          mutex;

    QWaitCondition  condVar;

    QTimer         *queryTimer;

    KProcess       *process;
};

KDcraw::KDcraw()
{
    d = new KDcrawPriv;
    m_cancel  = false;
}

KDcraw::~KDcraw()
{
    cancel();
    delete d;
}

void KDcraw::cancel()
{
    m_cancel = true;
}

bool KDcraw::loadDcrawPreview(QImage& image, const QString& path)
{
    // In first, try to extrcat the embedded JPEG preview. Very fast.
    bool ret = loadEmbeddedPreview(image, path);
    if (ret) return true;

    // In second, decode and half size of RAW picture. More slow.
    return (loadHalfPreview(image, path));
}

bool KDcraw::loadEmbeddedPreview(QImage& image, const QString& path)
{
    QByteArray  imgData;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
        return false;

    // Try to extract embedded thumbnail using dcraw with options:
    // -c : write to stdout
    // -e : Extract the camera-generated thumbnail, not the raw image (JPEG or a PPM file).
    // Note : this code require at least dcraw version 8.x

    KProcess process;
    process << DcrawBinary::path();
    process << "-c" <<  "-e" << path;

    qDebug() << "Running RAW decoding command:" << process.program().join(" ");

    process.setOutputChannelMode(KProcess::SeparateChannels);
    process.setNextOpenMode(QIODevice::ReadOnly);
    process.start();

    while (process.waitForReadyRead(-1))
    {
        imgData.append(process.readAllStandardOutput());
    }
    process.waitForFinished();

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
        {
            qDebug() << "Using embedded RAW preview extraction";
            return true;
        }
    }

    return false;
}

bool KDcraw::loadHalfPreview(QImage& image, const QString& path)
{
    QByteArray  imgData;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
        return false;

    // Try to use simple RAW extraction method in 8 bits ppm output.
    // -c : write to stdout
    // -h : Half-size color image (3x faster than -q)
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible

    KProcess process;
    process << DcrawBinary::path();
    process << "-c" << "-h" << "-w" << "-a";
    process << path;

    qDebug() << "Running RAW decoding command:" << process.program().join(" ");

    process.setOutputChannelMode(KProcess::SeparateChannels);
    process.setNextOpenMode(QIODevice::ReadOnly);
    process.start();

    while (process.waitForReadyRead(-1))
    {
        imgData.append(process.readAllStandardOutput());
    }
    process.waitForFinished();

    if ( !imgData.isEmpty() )
    {
        if (image.loadFromData( imgData ))
        {
            qDebug("Using reduced RAW picture extraction");
            return true;
        }
    }

    return false;
}

bool KDcraw::rawFileIdentify(DcrawInfoContainer& identify, const QString& path)
{
    FILE       *f=NULL;
    QByteArray  txtData;
    const int   MAX_IPC_SIZE = (1024*32);
    char        buffer[MAX_IPC_SIZE];
    QFile       file;
    qint64      len;
    QByteArray  command;

    QFileInfo fileInfo(path);
    QString   rawFilesExt(raw_file_extentions);
    QString ext = fileInfo.suffix().toUpper();

    if (!fileInfo.exists() || ext.isEmpty() || !rawFilesExt.toUpper().contains(ext))
        return false;

    // Try to get camera maker/model using dcraw with options:
    // -i : identify files without decoding them.
    // -v : verbose mode.

    command  = DcrawBinary::path();
    command += " -i -v ";
    command += QFile::encodeName( KShell::quoteArg( path ) );
    qDebug("Running RAW decoding command: %s", (const char*)command);

    f = popen( command.data(), "r" );

    if ( f == NULL )
    {
        identify = DcrawInfoContainer();
        return false;
    }

    file.open(f, QIODevice::ReadOnly);

    while ((len = file.read(buffer, MAX_IPC_SIZE)) != 0)
    {
        if ( len == -1 )
        {
            identify = DcrawInfoContainer();
            return false;
        }
        else
        {
            int oldSize = txtData.size();
            txtData.resize( txtData.size() + len );
            memcpy(txtData.data()+oldSize, buffer, len);
        }
    }

    file.close();
    pclose( f );
    QString dcrawInfo(txtData);

    if ( dcrawInfo.isEmpty() )
    {
        identify = DcrawInfoContainer();
        return false;
    }

    int pos;

    // Extract Time Stamp.
    QString timeStampHeader("Timestamp: ");
    pos = dcrawInfo.indexOf(timeStampHeader);
    if (pos != -1)
    {
        QString timeStamp = dcrawInfo.mid(pos).section('\n', 0, 0);
        timeStamp.remove(0, timeStampHeader.length());
        identify.dateTime = QDateTime::fromString(timeStamp);
    }

    // Extract Camera Maker.
    QString makeHeader("Camera: ");
    pos = dcrawInfo.indexOf(makeHeader);
    if (pos != -1)
    {
        QString make = dcrawInfo.mid(pos).section('\n', 0, 0);
        make.remove(0, makeHeader.length());
        identify.make = make;
    }

    // Extract Camera Model.
    QString modelHeader("Model: ");
    pos = dcrawInfo.indexOf(modelHeader);
    if (pos != -1)
    {
        QString model = dcrawInfo.mid(pos).section('\n', 0, 0);
        model.remove(0, modelHeader.length());
        identify.model = model;
    }

    // Extract Camera Model.
    QString ownerHeader("Owner: ");
    pos = dcrawInfo.indexOf(ownerHeader);
    if (pos != -1)
    {
        QString owner = dcrawInfo.mid(pos).section('\n', 0, 0);
        owner.remove(0, ownerHeader.length());
        identify.owner = owner;
    }

    // Extract DNG Version.
    QString DNGVersionHeader("DNG Version: ");
    pos = dcrawInfo.indexOf(DNGVersionHeader);
    if (pos != -1)
    {
        QString DNGVersion = dcrawInfo.mid(pos).section('\n', 0, 0);
        DNGVersion.remove(0, DNGVersionHeader.length());
        identify.DNGVersion = DNGVersion;
    }

    // Extract ISO Speed.
    QString isoSpeedHeader("ISO speed: ");
    pos = dcrawInfo.indexOf(isoSpeedHeader);
    if (pos != -1)
    {
        QString isoSpeed = dcrawInfo.mid(pos).section('\n', 0, 0);
        isoSpeed.remove(0, isoSpeedHeader.length());
        identify.sensitivity = isoSpeed.toLong();
    }

    // Extract Shutter Speed.
    QString shutterSpeedHeader("Shutter: ");
    pos = dcrawInfo.indexOf(shutterSpeedHeader);
    if (pos != -1)
    {
        QString shutterSpeed = dcrawInfo.mid(pos).section('\n', 0, 0);
        shutterSpeed.remove(0, shutterSpeedHeader.length());

        if (shutterSpeed.startsWith("1/"))
            shutterSpeed.remove(0, 2);                   // remove "1/" at start of string.

        shutterSpeed.remove(shutterSpeed.length()-4, 4); // remove " sec" at end of string.
        identify.exposureTime = shutterSpeed.toFloat();
    }

    // Extract Aperture.
    QString apertureHeader("Aperture: f/");
    pos = dcrawInfo.indexOf(apertureHeader);
    if (pos != -1)
    {
        QString aperture = dcrawInfo.mid(pos).section('\n', 0, 0);
        aperture.remove(0, apertureHeader.length());
        identify.aperture = aperture.toFloat();
    }

    // Extract Focal Length.
    QString focalLengthHeader("Focal Length: ");
    pos = dcrawInfo.indexOf(focalLengthHeader);
    if (pos != -1)
    {
        QString focalLength = dcrawInfo.mid(pos).section('\n', 0, 0);
        focalLength.remove(0, focalLengthHeader.length());
        focalLength.remove(focalLength.length()-3, 3);    // remove " mm" at end of string.
        identify.focalLength = focalLength.toFloat();
    }

    // Extract Image Size.

    QString imageSizeHeader("Image size:  ");
    pos = dcrawInfo.indexOf(imageSizeHeader);
    if (pos != -1)
    {
        QString imageSize = dcrawInfo.mid(pos).section('\n', 0, 0);
        imageSize.remove(0, imageSizeHeader.length());
        int width  = imageSize.section(" x ", 0, 0).toInt();
        int height = imageSize.section(" x ", 1, 1).toInt();
        identify.imageSize = QSize(width, height);
    }

    // Extract "Has an embedded ICC profile" flag.

    QString hasIccProfileHeader("Embedded ICC profile: ");
    pos = dcrawInfo.indexOf(hasIccProfileHeader);
    if (pos != -1)
    {
        QString hasIccProfile = dcrawInfo.mid(pos).section('\n', 0, 0);
        hasIccProfile.remove(0, hasIccProfileHeader.length());
        if (hasIccProfile.contains("yes"))
            identify.hasIccProfile = true;
        else
            identify.hasIccProfile = false;
    }

    // Check if picture is decodable

    identify.isDecodable = true;
    pos = dcrawInfo.indexOf("Cannot decode file");
    if (pos != -1)
        identify.isDecodable = false;

    // Extract "Has Secondary Pixel" flag.

    QString hasSecondaryPixelHeader("Secondary pixels: ");
    pos = dcrawInfo.indexOf(hasSecondaryPixelHeader);
    if (pos != -1)
    {
        QString hasSecondaryPixel = dcrawInfo.mid(pos).section('\n', 0, 0);
        hasSecondaryPixel.remove(0, hasSecondaryPixelHeader.length());
        if (hasSecondaryPixel.contains("yes"))
            identify.hasSecondaryPixel = true;
        else
            identify.hasSecondaryPixel = false;
    }

    // Extract Pixel Aspect Ratio.
    QString aspectRatioHeader("Pixel Aspect Ratio: ");
    pos = dcrawInfo.indexOf(aspectRatioHeader);
    if (pos != -1)
    {
        QString aspectRatio = dcrawInfo.mid(pos).section('\n', 0, 0);
        aspectRatio.remove(0, aspectRatioHeader.length());
        identify.pixelAspectRatio = aspectRatio.toFloat();
    }

    // Extract Raw Colors.
    QString rawColorsHeader("Raw colors: ");
    pos = dcrawInfo.indexOf(rawColorsHeader);
    if (pos != -1)
    {
        QString rawColors = dcrawInfo.mid(pos).section('\n', 0, 0);
        rawColors.remove(0, rawColorsHeader.length());
        identify.rawColors = rawColors.toInt();
    }

    // Extract Raw Images.
    QString rawImagesHeader("Number of raw images: ");
    pos = dcrawInfo.indexOf(rawImagesHeader);
    if (pos != -1)
    {
        QString rawImages = dcrawInfo.mid(pos).section('\n', 0, 0);
        rawImages.remove(0, rawImagesHeader.length());
        identify.rawImages = rawImages.toInt();
    }

    // Extract Filter Pattern.
    QString filterHeader("Filter pattern: ");
    pos = dcrawInfo.indexOf(filterHeader);
    if (pos != -1)
    {
        QString filter = dcrawInfo.mid(pos).section('\n', 0, 0);
        filter.remove(0, filterHeader.length());
        identify.filterPattern = filter;
    }

    // Extract Daylight Multipliers.
    QString daylightMultHeader("Daylight multipliers: ");
    pos = dcrawInfo.indexOf(daylightMultHeader);
    if (pos != -1)
    {
        QString daylightMult = dcrawInfo.mid(pos).section('\n', 0, 0);
        daylightMult.remove(0, daylightMultHeader.length());
        identify.daylightMult[0] = daylightMult.section(" ", 0, 0).toDouble();
        identify.daylightMult[1] = daylightMult.section(" ", 1, 1).toDouble();
        identify.daylightMult[2] = daylightMult.section(" ", 2, 2).toDouble();
    }

    // Extract Camera Multipliers.
    QString cameraMultHeader("Camera multipliers: ");
    pos = dcrawInfo.indexOf(cameraMultHeader);
    if (pos != -1)
    {
        QString cameraMult = dcrawInfo.mid(pos).section('\n', 0, 0);
        cameraMult.remove(0, cameraMultHeader.length());
        identify.cameraMult[0] = cameraMult.section(" ", 0, 0).toDouble();
        identify.cameraMult[1] = cameraMult.section(" ", 1, 1).toDouble();
        identify.cameraMult[2] = cameraMult.section(" ", 2, 2).toDouble();
        identify.cameraMult[3] = cameraMult.section(" ", 3, 3).toDouble();
    }

    return true;
}

// ----------------------------------------------------------------------------------

bool KDcraw::decodeHalfRAWImage(const QString& filePath, RawDecodingSettings rawDecodingSettings, 
                                QByteArray &imageData, int &width, int &height, int &rgbmax)
{
    m_rawDecodingSettings                    = rawDecodingSettings;
    m_rawDecodingSettings.halfSizeColorImage = true;
    return (loadFromDcraw(filePath, imageData, width, height, rgbmax));
}

bool KDcraw::decodeRAWImage(const QString& filePath, RawDecodingSettings rawDecodingSettings, 
                            QByteArray &imageData, int &width, int &height, int &rgbmax)
{
    m_rawDecodingSettings = rawDecodingSettings;
    return (loadFromDcraw(filePath, imageData, width, height, rgbmax));
}

bool KDcraw::checkToCancelWaitingData()
{
    return m_cancel;
}

bool KDcraw::checkToCancelRecievingData()
{
    return m_cancel;
}

void KDcraw::setWaitingDataProgress(double)
{
}

void KDcraw::setRecievingDataProgress(double)
{
}

bool KDcraw::loadFromDcraw(const QString& filePath, QByteArray &imageData, 
                           int &width, int &height, int &rgbmax)
{
    m_cancel      = false;
    d->dataPos     = 0;
    d->filePath   = filePath;
    d->process    = 0;
    d->data       = 0;
    d->width      = 0;
    d->height     = 0;
    d->rgbmax     = 0;

    if (!startProcess())
        return false;

    // The time from starting dcraw to when it first outputs something takes
    // much longer than the time while it outputs the data and the time while
    // we process the data.
    // We do not have progress information for this, but it is much more promising to the user
    // if there is progress which does not stay at a fixed value.
    // So we make up some progress (0% - 40%), using the file size as an indicator how long it might take.
    QTime dcrawStartTime = QTime::currentTime();
    int fileSize         = QFileInfo(filePath).size();
    
    // This is the magic number that describes how fast the function grows
    // It _should_ be dependent on how fast the computer is, but we do not have this piece of information
    // So this is a number that works well on my computer.
    double K50         = 3000.0*fileSize;
    double part        = 0;
    int checkpointTime = 0;
    int checkpoint     = 0;

    while (d->process->state() == QProcess::Running &&
           !( (d->dataPos == 0) ? checkToCancelWaitingData() : checkToCancelRecievingData() ) )
    {
        if (d->dataPos == 0)
        {
            int elapsedMsecs = dcrawStartTime.msecsTo(QTime::currentTime());
            if (elapsedMsecs > checkpointTime)
                checkpointTime += 300;

            // What we do here is a sigmoidal curve, it starts slowly,
            // then grows more rapidly, slows down again and
            // get asymptotically closer to the maximum.
            // (this is the Hill Equation, 2.8 the Hill Coefficient, to pour some blood in this)
            double elapsedMsecsPow = pow(elapsedMsecs, 2.8);
            part = (elapsedMsecsPow) / (K50 + elapsedMsecsPow);

            // While we waiting to receive data, progress from 0% to 40%
            setWaitingDataProgress(0.4*part);
        }
        else if (d->dataPos > checkpoint)
        {
            // While receiving data, progress from 40% to 70%
            double delta = 0.3 + 0.4 - 0.4*part;
            int imageSize = d->width * d->height * (m_rawDecodingSettings.sixteenBitsImage ? 6 : 3);
            checkpoint += (int)(imageSize / (20 * delta));
            setRecievingDataProgress(0.4*part + delta * (((float)d->dataPos)/((float)imageSize))); 
        }

        //QMutexLocker lock(&d->mutex);
        //d->condVar.wait(&d->mutex, 10);
        d->process->setReadChannel(QProcess::StandardOutput);
        if (d->process->waitForReadyRead(25))
            readData();
        d->process->setReadChannel(QProcess::StandardError);
        if (d->process->bytesAvailable())
            readErrorData();
    }

    if (d->process->state() == QProcess::Running)
        d->process->kill();

    bool normalExit = d->process->exitStatus() == QProcess::NormalExit && d->process->exitCode() == 0;
    delete d->process;
    d->process    = 0;

    if (!normalExit || m_cancel)
    {
        delete [] d->data;
        d->data = 0;
        return false;
    }

    // Copy decoded image data to byte array.
    width     = d->width;
    height    = d->height;
    rgbmax    = d->rgbmax;
    imageData = QByteArray(d->data, d->width * d->height * (m_rawDecodingSettings.sixteenBitsImage ? 6 : 3));

    delete [] d->data;
    d->data = 0;

    return true;
}

bool KDcraw::startProcess()
{
    if (checkToCancelWaitingData())
    {
        return false;
    }

    // create KProcess and build argument list

    d->process = new KProcess;

    // run dcraw with options:
    // -c : write to stdout
    // -v : verboze mode.
    //
    // -4 : 16bit ppm output
    //
    // -f : Interpolate RGB as four colors. This blurs the image a little, but it eliminates false 2x2 mesh patterns.
    // -a : Use automatic white balance
    // -w : Use camera white balance, if possible
    // -n : Use wavelets to erase noise while preserving real detail. 
    // -j : Do not stretch the image to its correct aspect ratio.
    // -q : Use an interpolation method.
    // -p : Use the input ICC profiles to define the camera's raw colorspace.
    // -o : Use ICC profiles to define the output colorspace.
    // -h : Output a half-size color image. Twice as fast as -q 0.
    // -b : set Brightness value.
    // -k : set Black Point value.
    // -r : set Raw Color Balance Multipliers.

    QStringList args;
    args << "-c";
    args << "-v";

    if (m_rawDecodingSettings.sixteenBitsImage)
        args << "-4";

    if (m_rawDecodingSettings.halfSizeColorImage)
        args << "-h";

    if (m_rawDecodingSettings.cameraColorBalance)
        args << "-w";

    if (m_rawDecodingSettings.automaticColorBalance)
        args << "-a";

    if (m_rawDecodingSettings.RGBInterpolate4Colors)
        args << "-f";

    if (m_rawDecodingSettings.DontStretchPixels)
        args << "-j";

    args << "-H";
    args << QString::number(m_rawDecodingSettings.unclipColors);

    args << "-b";
    args << QString::number(m_rawDecodingSettings.brightness);

    if (m_rawDecodingSettings.enableBlackPoint)
    {
        args << "-k";
        args << QString::number(m_rawDecodingSettings.blackPoint);
    }

    if (m_rawDecodingSettings.enableColorMultipliers)
    {
        args << "-r";
        args << QString::number(m_rawDecodingSettings.colorBalanceMultipliers[0], 'f', 5);
        args << QString::number(m_rawDecodingSettings.colorBalanceMultipliers[1], 'f', 5);
        args << QString::number(m_rawDecodingSettings.colorBalanceMultipliers[2], 'f', 5);
        args << QString::number(m_rawDecodingSettings.colorBalanceMultipliers[3], 'f', 5);
    }

    args << "-q";
    args << QString::number(m_rawDecodingSettings.RAWQuality);

    if (m_rawDecodingSettings.enableNoiseReduction)
    {
        args << "-n";
        args << QString::number(m_rawDecodingSettings.NRThreshold);
    }

    args << "-o";
    args << QString::number(m_rawDecodingSettings.outputColorSpace);

    args << QFile::encodeName(d->filePath);

    QString command = QString::fromAscii(DcrawBinary::path());
    command += args.join(" ");

    qDebug("Running RAW decoding command: %s", command.toAscii().constData());

    // actually start the process
    d->process->setProgram(QString::fromAscii(DcrawBinary::path()), args);
    d->process->setOutputChannelMode(KProcess::SeparateChannels);
    d->process->setNextOpenMode(QIODevice::ReadOnly);
    d->process->start();

    while (d->process->state() == QProcess::Starting)
    {
        d->process->waitForStarted(10);
    }

    if (d->process->state() == QProcess::NotRunning)
    {
        qWarning("Failed to start RAW decoding");
        delete d->process;
        d->process    = 0;
        return false;
    }
    return true;
}

void KDcraw::readData()
{
    QByteArray data = d->process->readAll();
    const char *buffer = data.constData();
    int buflen = data.length();

    if (!d->data)
    {
        // first data packet:
        // Parse PPM header to find out size and allocate buffer

        // PPM header is "P6 <width> <height> <maximum rgb value "
        // where the blanks are newline characters

        QString magic = QString::fromAscii(buffer, 2);
        if (magic != "P6") 
        {
            qWarning("Cannot parse header from RAW decoding: Magic is: %s", magic.toAscii().constData());
            d->process->kill();
            return;
        }

        // Find the third newline that marks the header end in a dcraw generated ppm.
        int i       = 0;
        int counter = 0;

        while (i < buflen) 
        {
            if (counter == 3) break;
            if (buffer[i] == '\n') 
            {
                counter++;
            }
            ++i;
        }

        QString temp = QString::fromAscii(buffer, i);
        QStringList splitlist = temp.split(QString("\n"));
        temp = splitlist[1];
        QStringList sizes = temp.split(QString(" "));
        if (splitlist.size() < 3 || sizes.size() < 2)
        {
            qWarning("Cannot parse header from RAW decoding: Could not split");
            d->process->kill();
            return;
        }

        d->width  = sizes[0].toInt();
        d->height = sizes[1].toInt();
        d->rgbmax = splitlist[2].toInt();

#ifdef ENABLE_DEBUG_MESSAGES
        qDebug("Parsed PPM header: width %i height %i rgbmax %i", d->width, d->height, d->rgbmax);
#endif

        // cut header from data for memcpy below
        buffer += i;
        buflen -= i;

        // allocate buffer
        d->data    = new char[d->width * d->height * (m_rawDecodingSettings.sixteenBitsImage ? 6 : 3)];
        d->dataPos = 0;
    }

    // copy data to buffer
    memcpy(d->data + d->dataPos, buffer, buflen);
    d->dataPos += buflen;
}

void KDcraw::readErrorData()
{
    QByteArray message = d->process->readAllStandardError();
    qDebug("RAW decoding StdErr: %s", (const char*)message);
}

}  // namespace KDcrawIface
