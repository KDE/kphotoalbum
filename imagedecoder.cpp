#include "imagedecoder.h"

QPtrList<ImageDecoder>* ImageDecoder::decoders()
{
	static QPtrList<ImageDecoder> s_decoders;
	return &s_decoders;
}

ImageDecoder::ImageDecoder()
{
	decoders()->append(this);
}

ImageDecoder::~ImageDecoder()
{
	decoders()->remove(this);
}

bool ImageDecoder::decode(QImage *img, const QString& imageFile, QSize* fullSize, int dim)
{
	QPtrList<ImageDecoder>* lst = decoders();
	for( QPtrList<ImageDecoder>::const_iterator it = lst->begin(); it != lst->end(); ++it ) {
		if( (*it)->_decode(img,imageFile,fullSize,dim) ) return true;
	}
	return false;
}

bool ImageDecoder::mightDecode( const QString& imageFile )
{
	QPtrList<ImageDecoder>* lst = decoders();
	for( QPtrList<ImageDecoder>::const_iterator it = lst->begin(); it != lst->end(); ++it ) {
		if( (*it)->_mightDecode(imageFile) ) return true;
	}
	return false;
}
