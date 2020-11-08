/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef FEATUREDIALOG_H
#define FEATUREDIALOG_H

#include <QDialog>
#include <QTextBrowser>

namespace MainWindow
{

class FeatureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FeatureDialog(QWidget *parent);

    QSize sizeHint() const override;

    static bool hasAllFeaturesAvailable();
    static QString featureString();
    static QStringList supportedVideoMimeTypes();
    static QString ffmpegBinary();
    static QString ffprobeBinary();
    /**
     * @brief hasVideoThumbnailer
     * @return true, if a program capable of creating video thumbnails is found, false otherwise
     */
    static bool hasVideoThumbnailer();
    /**
     * @brief hasVideoProber
     * @return  true, if a program capable of extracting video metadata is found, false otherwise
     */
    static bool hasVideoProber();

protected:
    static bool hasPurposeSupport();
    static bool hasEXIV2Support();
    static bool hasEXIV2DBSupport();
    static bool hasGeoMapSupport();
};
}

#endif /* FEATUREDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
