#include "ThumbnailComponent.h"
#include "ThumbnailFactory.h"

ThumbnailView::ThumbnailComponent::ThumbnailComponent( ThumbnailFactory* factory )
    :_factory( factory )
{
}

ThumbnailView::ThumbnailModel* ThumbnailView::ThumbnailComponent::model()
{
    return _factory->model();
}

ThumbnailView::CellGeometry* ThumbnailView::ThumbnailComponent::cellGeometryInfo()
{
    return _factory->cellGeometry();
}

ThumbnailView::ThumbnailWidget* ThumbnailView::ThumbnailComponent::widget()
{
    return _factory->widget();
}

ThumbnailView::ThumbnailPainter* ThumbnailView::ThumbnailComponent::painter()
{
    return _factory->painter();
}

const ThumbnailView::ThumbnailModel* ThumbnailView::ThumbnailComponent::model() const
{
    return _factory->model();
}

const ThumbnailView::CellGeometry* ThumbnailView::ThumbnailComponent::cellGeometryInfo() const
{
    return _factory->cellGeometry();
}

const ThumbnailView::ThumbnailWidget* ThumbnailView::ThumbnailComponent::widget() const
{
    return _factory->widget();
}

const ThumbnailView::ThumbnailPainter* ThumbnailView::ThumbnailComponent::painter() const
{
    return _factory->painter();
}

ThumbnailView::ThumbnailCache* ThumbnailView::ThumbnailComponent::cache()
{
    return _factory->cache();
}

const ThumbnailView::ThumbnailCache* ThumbnailView::ThumbnailComponent::cache() const
{
    return _factory->cache();
}
