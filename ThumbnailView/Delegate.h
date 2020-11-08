/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
