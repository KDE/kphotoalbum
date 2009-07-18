#ifndef THUMBNAILCOMPONENT_H
#define THUMBNAILCOMPONENT_H

namespace DB {class ResultId; }

namespace ThumbnailView
{
class ThumbnailFactory;
class ThumbnailPainter;
class ThumbnailWidget;
class CellGeometry;
class ThumbnailModel;
class ThumbnailCache;

class ThumbnailComponent
{
public:
    ThumbnailComponent( ThumbnailFactory* factory );

    ThumbnailModel* model();
    const ThumbnailModel* model() const;

    CellGeometry* cellGeometryInfo();
    const CellGeometry* cellGeometryInfo() const;

    ThumbnailWidget* widget();
    const ThumbnailWidget* widget() const;

    ThumbnailPainter* painter();
    const ThumbnailPainter* painter() const;

    ThumbnailCache* cache();
    const ThumbnailCache* cache() const;
private:
    ThumbnailFactory* _factory;
};

}

#endif /* THUMBNAILCOMPONENT_H */

