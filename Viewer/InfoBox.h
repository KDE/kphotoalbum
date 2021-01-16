/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOBOX_H
#define INFOBOX_H

#include "InfoBoxResizer.h"
#include "ViewerWidget.h"

#include <kpabase/SettingsData.h>
#include <kpabase/config-kpa-marble.h>

#include <QMouseEvent>
#include <QPointer>
#include <QTextBrowser>

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
    bool event(QEvent *e) override;
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
    void updatePalette();

protected slots:
    void jumpToContext();
    void linkHovered(const QString &linkName);
#ifdef HAVE_MARBLE
    void launchMapView();
    void updateMapForCurrentImage(DB::FileName);
#endif

signals:
    void tagHovered(const QPair<QString, QString> &tagData);
    void noTagHovered();

private: // Variables
    QMap<int, QPair<QString, QString>> m_linkMap;
    ViewerWidget *m_viewer;
    QToolButton *m_jumpToContext;
    bool m_hoveringOverLink;
    InfoBoxResizer m_infoBoxResizer;
    VisibleOptionsMenu *m_menu;
    QList<QPixmap> m_ratingPixmap;
#ifdef HAVE_MARBLE
    QToolButton *m_showOnMap;
    QPointer<Map::MapView> m_map;
#endif
};
}

#endif // INFOBOX_H

// vi:expandtab:tabstop=4 shiftwidth=4:
