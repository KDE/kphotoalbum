#ifndef THUMBNAILFACTORY_H
#define THUMBNAILFACTORY_H

namespace ThumbnailView
{
class ThumbnailWidget;
class CellGeometry;
class ThumbnailModel;
class ThumbnailPainter;
class ThumbnailCache;

class ThumbnailFactory
{
public:
    virtual ~ThumbnailFactory() {};
    virtual ThumbnailModel* model() = 0;
    virtual CellGeometry* cellGeometry() = 0;
    virtual ThumbnailWidget* widget() = 0;
    virtual ThumbnailPainter* painter() = 0;
    virtual ThumbnailCache* cache() = 0;
};

}

#endif /* THUMBNAILFACTORY_H */

