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

#ifndef CENTERINGICONVIEW_H
#define CENTERINGICONVIEW_H
#include <QListView>
class QTimer;

namespace Browser
{

/**
 * \brief QListView subclass which shows its content as icons centered in the middle
 *
 * To make KPhotoAlbum slightly more sexy, the overview page has its items
 * centered at the middle of the screen. This class has the capability to
 * do that when \ref setViewMode is called with \ref CenterView as argument.
 */
class CenteringIconView : public QListView
{
    Q_OBJECT
public:
    enum ViewMode { CenterView,
                    NormalIconView };

    explicit CenteringIconView(QWidget *parent);
    void setViewMode(ViewMode);
    void setModel(QAbstractItemModel *) override;
    void showEvent(QShowEvent *) override;

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void setupMargins();
    int columnCount(int elementCount) const;
    int availableWidth() const;
    int availableHeight() const;

private:
    ViewMode m_viewMode;
};

}

#endif /* CENTERINGICONVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
