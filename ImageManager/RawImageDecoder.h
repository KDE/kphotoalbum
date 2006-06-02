#include "ImageDecoder.h"

#include <qimage.h>
#include <qstring.h>
#include <qsize.h>

namespace ImageManager
{

class RAWImageDecoder : public ImageDecoder {
public:
	RAWImageDecoder() {}

	virtual bool _decode(QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1);
	virtual bool _mightDecode( const QString& imageFile );
};

}
