#ifndef IMAGEDECODER_H
#define IMAGEDECODER_H

#include <qptrlist.h>
#include <qimage.h>

namespace ImageManager
{

class ImageDecoder {
public:
	static bool decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1 );
	static bool mightDecode( const QString& imageFile );

	virtual ~ImageDecoder();

protected:
	ImageDecoder();
	virtual bool _decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1 ) = 0;
	virtual bool _mightDecode( const QString& imageFile ) = 0;

private:
	static QPtrList<ImageDecoder>* decoders();
};
}

#endif /* IMAGEDECODER_H */

