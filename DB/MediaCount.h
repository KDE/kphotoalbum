#ifndef MEDIACOUNT_H
#define MEDIACOUNT_H

namespace DB
{
class MediaCount
{
public:
    MediaCount() : _images(0), _videos(0) {}
    MediaCount( int images, int videos ) : _images( images ), _videos( videos ) {}
    int images() const { return _images; }
    int videos() const { return _videos; }
    int total() const { return _images + _videos; }

private:
    int _images;
    int _videos;
};

}

#endif /* MEDIACOUNT_H */

