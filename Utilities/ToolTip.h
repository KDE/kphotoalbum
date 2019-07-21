/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UTILITIES_TOOLTIP_H
#define UTILITIES_TOOLTIP_H

#include <DB/FileName.h>
#include <ImageManager/ImageClientInterface.h>

#include <QLabel>
class QTemporaryFile;

namespace Utilities
{

class ToolTip : public QLabel, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    explicit ToolTip(QWidget *parent = nullptr, Qt::WindowFlags f = 0);
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    void requestToolTip(const DB::FileName &fileName);

protected:
    virtual void placeWindow() = 0;

private:
    void renderToolTip();
    void requestImage(const DB::FileName &fileName);
    DB::FileName m_currentFileName;
    QTemporaryFile *m_tmpFileForThumbnailView;
};

} // namespace Utilities

#endif // UTILITIES_TOOLTIP_H
// vi:expandtab:tabstop=4 shiftwidth=4:
