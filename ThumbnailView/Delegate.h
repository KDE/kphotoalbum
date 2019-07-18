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
#ifndef DELEGATE_H
#define DELEGATE_H
#include "ThumbnailComponent.h"
#include <DB/ImageInfoPtr.h>
#include <QStyledItemDelegate>

namespace ThumbnailView
{

class Delegate : public QStyledItemDelegate, private ThumbnailComponent
{
public:
    explicit Delegate(ThumbnailFactory *factory, QObject *parent);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintCellBackground(QPainter *painter, const QRect &rect) const;
    void paintCellPixmap(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintVideoInfo(QPainter *painter, const QRect &pixmapRect, const QModelIndex &index) const;
    void paintCellText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintBoundingRect(QPainter *painter, const QRect &pixmapRect, const QModelIndex &index) const;
    void paintStackedIndicator(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void paintDropIndicator(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    bool isFirst(int row) const;
    bool isLast(int row) const;

    QString videoLengthText(const DB::ImageInfoPtr &imageInfo) const;
};

}

#endif /* DELEGATE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
