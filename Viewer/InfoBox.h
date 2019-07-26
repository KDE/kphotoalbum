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

#ifndef INFOBOX_H
#define INFOBOX_H

#include "config-kpa-kgeomap.h"

// Qt includes
#include <QMouseEvent>
#include <QPointer>

// KDE includes
#include <QTextBrowser>

// Local includes
#include "InfoBoxResizer.h"
#include "ViewerWidget.h"

#include <Settings/SettingsData.h>

// Qt classes
class QMenu;
class QToolButton;

namespace Map
{

// Local classes
class MapView;

}

namespace Viewer
{

// Local classes
class VisibleOptionsMenu;

class InfoBox : public QTextBrowser
{
    Q_OBJECT

public:
    explicit InfoBox(ViewerWidget *parent);
    void setSource(const QUrl &source) override;
    void setInfo(const QString &text, const QMap<int, QPair<QString, QString>> &linkMap);
    void setSize();

protected:
    QVariant loadResource(int type, const QUrl &name) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void updateCursor(const QPoint &pos);
    bool atBlackoutPos(bool left, bool right, bool top, bool bottom, Settings::Position windowPos) const;
    void showBrowser();
    void possiblyStartResize(const QPoint &pos);
    void hackLinkColorForQt52();

protected slots:
    void jumpToContext();
    void linkHovered(const QString &linkName);
#ifdef HAVE_KGEOMAP
    void launchMapView();
    void updateMapForCurrentImage(DB::FileName);
#endif

signals:
    void tagHovered(QPair<QString, QString> tagData);
    void noTagHovered();

private: // Variables
    QMap<int, QPair<QString, QString>> m_linkMap;
    ViewerWidget *m_viewer;
    QToolButton *m_jumpToContext;
    bool m_hoveringOverLink;
    InfoBoxResizer m_infoBoxResizer;
    VisibleOptionsMenu *m_menu;
    QList<QPixmap> m_ratingPixmap;
#ifdef HAVE_KGEOMAP
    QToolButton *m_showOnMap;
    QPointer<Map::MapView> m_map;
#endif
};
}

#endif // INFOBOX_H

// vi:expandtab:tabstop=4 shiftwidth=4:
