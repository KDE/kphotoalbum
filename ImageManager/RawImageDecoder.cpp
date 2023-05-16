// SPDX-FileCopyrightText: 2005 Steffen Hansen <hansen@kde.org>
// SPDX-FileCopyrightText: 2005-2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2006-2009 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007-2012 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2010 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012 Rex Dieter <rdieter@math.unl.edu>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2016 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2018 Robert Krawitz <rlk@alum.mit.edu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RawImageDecoder.h"

#include "ImageRequest.h"

#include <Utilities/FastJpeg.h>
#include <kpabase/FileExtensions.h>
#include <kpabase/FileName.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpabase/config-kpa-kdcraw.h>

#include <QFile>
#include <QImage>
#ifdef HAVE_KDCRAW
#include <KDCRAW/KDcraw>
#include <libkdcraw_version.h>
#endif

namespace ImageManager
{

bool RAWImageDecoder::_decode(QImage *img, ImageRequest *request, QSize *fullSize, int dim)
{
    /* width and height seem to be only hints, ignore */
    Q_UNUSED(dim)

#ifdef HAVE_KDCRAW
    QSize size;
    QSize *fSize = (fullSize) ? fullSize : &size;
    const DB::FileName &imageFile = request->fileSystemFileName();
    QByteArray previewData;
    if (!KDcrawIface::KDcraw::loadEmbeddedPreview(previewData, imageFile.absolute()))
        return false;

    // Faster than allowing loadRawPreview to do the decode itself
    if (!Utilities::loadJPEG(img, previewData, fSize, dim))
        return false;

    qCDebug(ImageManagerLog) << "Got embedded preview for raw file" << imageFile.relative();
    qCDebug(ImageManagerLog) << "  Preview size:" << img->width() << "x" << img->height();
    qCDebug(ImageManagerLog) << "  Requested dimension:" << dim;
    qCDebug(ImageManagerLog) << "  useRawThumbnail:" << Settings::SettingsData::instance()->useRawThumbnail();
    qCDebug(ImageManagerLog) << "  useRawThumbnailSize:" << Settings::SettingsData::instance()->useRawThumbnailSize();

    if (Settings::SettingsData::instance()->useRawThumbnail()
        && ((dim > 0 && img->width() >= dim && img->height() >= dim)
            || (img->width() >= Settings::SettingsData::instance()->useRawThumbnailSize().width()
                && img->height() >= Settings::SettingsData::instance()->useRawThumbnailSize().height()))) {
        qCDebug(ImageManagerLog) << "Preferring embedded raw thumbnail...";
        return true;
    }

    KDcrawIface::DcrawInfoContainer metadata;
    if (!KDcrawIface::KDcraw::rawFileIdentify(metadata, imageFile.absolute()))
        return false;
    qCDebug(ImageManagerLog) << "  Raw image size:" << metadata.imageSize;
    qCDebug(ImageManagerLog) << "  Raw image orientation enum:" << metadata.orientation;
    if ((img->width() < metadata.imageSize.width() * 0.8) || (img->height() < metadata.imageSize.height() * 0.8)) {

        qCDebug(ImageManagerLog) << "Decoding the raw image is required...";
        // let's try to get a better resolution
        KDcrawIface::KDcraw decoder;
        KDcrawIface::RawDecodingSettings rawDecodingSettings;

        if (rawDecodingSettings.sixteenBitsImage) {
            qCWarning(ImageManagerLog) << "16 bits per color channel is not supported yet";
            return false;
        } else {
            QByteArray imageData; /* 3 bytes for each pixel,  */
            int width, height, rgbmax;
            if (!decoder.decodeRAWImage(imageFile.absolute(), rawDecodingSettings, imageData, width, height, rgbmax))
                return false;

            // Now the funny part, how to turn this fugly QByteArray into an QImage. Yay!
            *img = QImage(width, height, QImage::Format_RGB32);
            if (img->isNull())
                return false;

            uchar *data = img->bits();

            for (int i = 0; i < imageData.size(); i += 3, data += 4) {
                data[0] = imageData[i + 2]; // blue
                data[1] = imageData[i + 1]; // green
                data[2] = imageData[i]; // red
                data[3] = 0xff; // alpha
            }
        }
        // The preview data for raw images is always returned in its non-rotated form by libkdcraw,
        // but the raw image itself is returned in its rotated form.
        // For decoded raw images, we therefore need to tell the ImageLoaderThread not to rotate the image a second time.
        request->setImageIsPreRotated(true);

    } else
        qCDebug(ImageManagerLog) << "Embedded raw thumbnail is sufficient...";

    *fSize = img->size();

    return true;
#else /* HAVE_KDCRAW */
    Q_UNUSED(img)
    Q_UNUSED(fullSize)
    Q_UNUSED(request)
    return false;
#endif /* HAVE_KDCRAW */
}

bool RAWImageDecoder::_mightDecode(const DB::FileName &imageFile)
{
    const auto pref = Settings::SettingsData::instance()->skipRawIfOtherMatches() ? KPABase::FileTypePreference::PreferNonRawFile : KPABase::FileTypePreference::NoPreference;
    return KPABase::isUsableRawImage(imageFile, pref);
}

}
// vi:expandtab:tabstop=4 shiftwidth=4:
