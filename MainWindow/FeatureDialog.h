/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
    static bool hasKIPISupport();
    static bool hasEXIV2Support();
    static bool hasEXIV2DBSupport();
    static bool hasGeoMapSupport();
};
}

#endif /* FEATUREDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
