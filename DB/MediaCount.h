#ifndef MEDIACOUNT_H
#define MEDIACOUNT_H

namespace DB
{
class MediaCount
{
public:
    MediaCount() : _images(0), _movies(0) {}
    MediaCount( int images, int movies ) : _images( images ), _movies( movies ) {}
    int images() const { return _images; }
    int movies() const { return _movies; }
    int total() const { return _images + _movies; }

private:
    int _images;
    int _movies;
};

}

#endif /* MEDIACOUNT_H */

