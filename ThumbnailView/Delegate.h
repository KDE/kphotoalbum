#ifndef DELEGATE_H
#define DELEGATE_H
#include "ThumbnailComponent.h"
#include <QStyledItemDelegate>

namespace ThumbnailView
{

class Delegate :public QStyledItemDelegate, private ThumbnailComponent
{
public:
    Delegate( ThumbnailFactory* factory );
    OVERRIDE void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    OVERRIDE QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;

private:
    void paintCellBackground( QPainter* painter, const QRect& rect ) const;
    void paintCellPixmap( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    void paintCellText( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    void paintBoundingRect( QPainter* painter, const QRect& pixmapRect, const QModelIndex& index  ) const;
    void paintStackedIndicator( QPainter* painter, const QRect &rect, const QModelIndex& index ) const;
    void paintDropIndicator( QPainter* painter, const QRect& rect, const QModelIndex& index ) const;
    bool isFirst( int row ) const;
    bool isLast( int row ) const;
};

}

#endif /* DELEGATE_H */

