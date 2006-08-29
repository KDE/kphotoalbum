#ifndef MEDIACOUNT_H
#define MEDIACOUNT_H

namespace DB
{
class MediaCount
{
public:
    MediaCount() : _null(true), _images(0), _videos(0)  {}
    MediaCount( uint images, uint videos ) : _null(false), _images( images ), _videos( videos ) {}
    bool isNull() const { return _null; }
    uint images() const { return _images; }
    uint videos() const { return _videos; }
    uint total() const { return _images + _videos; }

private:
    bool _null;
    uint _images;
    uint _videos;
};

}

#endif /* MEDIACOUNT_H */

