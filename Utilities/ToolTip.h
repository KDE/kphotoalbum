/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UTILITIES_TOOLTIP_H
#define UTILITIES_TOOLTIP_H

#include <DB/FileName.h>
#include <ImageManager/ImageClientInterface.h>

#include <QLabel>
class QTemporaryFile;

namespace Utilities
{

/**
 * @brief The ToolTip class acts as a base class for tooltips that can show image thumbnails or image info text.
 * The subclasses only customize the window behaviour, such as placement and window flags.
 */
class ToolTip : public QLabel, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    void requestToolTip(const DB::FileName &fileName);

protected:
    bool event(QEvent *e) override;
    explicit ToolTip(QWidget *parent, Qt::WindowFlags f);
    virtual void placeWindow() = 0;
    void updatePalette();

private:
    void renderToolTip();
    void requestImage(const DB::FileName &fileName);
    DB::FileName m_currentFileName;
    QTemporaryFile *m_tmpFileForThumbnailView;
};

} // namespace Utilities

#endif // UTILITIES_TOOLTIP_H
// vi:expandtab:tabstop=4 shiftwidth=4:
