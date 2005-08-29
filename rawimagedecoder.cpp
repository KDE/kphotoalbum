#include "imagedecoder.h"

#include <ktempfile.h>
#include <qfile.h>
#include <qimage.h>
#include <qwmatrix.h>
#include <cstdio>

/* Main entry point into raw parser */
extern "C" {
	int extract_thumbnail( FILE*, FILE*, int* );
}

class RAWImageDecoder : public ImageDecoder {
public:
	RAWImageDecoder() {}

	virtual bool _decode(QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1);
	virtual bool _mightDecode( const QString& imageFile );
};

bool RAWImageDecoder::_decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim)
{
  /* width and height seem to be only hints, ignore */
  Q_UNUSED( dim );
  /* Open file and extract thumbnail */
  FILE* input = fopen( QFile::encodeName(imageFile), "rb" );
  if( !input ) return false;
  KTempFile output;
  output.setAutoDelete(true);
  int orientation = 0;
  if( extract_thumbnail( input, output.fstream(), &orientation ) ) {
	fclose(input);
	return false;
  }
  fclose(input);
  output.close();
  if( !img->load( output.name() ) ) return false;

  if( fullSize ) *fullSize = img->size();

  if(orientation) {
	QWMatrix M;
	QWMatrix flip= QWMatrix(-1,0,0,1,0,0);
	switch(orientation+1) {  // notice intentional fallthroughs
	case 2: M = flip; break;
	case 4: M = flip;
	case 3: M.rotate(180); break;
	case 5: M = flip;
	case 6: M.rotate(90); break;
	case 7: M = flip;
	case 8: M.rotate(270); break;
	default: break; // should never happen
	}
	*img = img->xForm(M);
  }
  return true;
}

/* TODO(steffen): What is the input for this function?
   It seems that is is _not_ the full filename...
*/
bool RAWImageDecoder::_mightDecode( const QString& imageFile )
{
	/* Known RAW file extensions. TODO: Complete */
	static const QString extensions[] = { QString::fromLatin1("crw"),
										  QString::fromLatin1("cr2"),
										  QString::fromLatin1("nef"),
										  QString::fromLatin1("bay"),
										  QString::fromLatin1("mos"),
										  QString::fromLatin1("mrw"),
										  QString::fromLatin1("orf"),
										  QString::fromLatin1("cs1"),
										  QString::fromLatin1("dc2"),
										  QString::fromLatin1("kdc"),
										  QString::fromLatin1("raf"),
										  QString::fromLatin1("rdc"),
										  QString::fromLatin1("x3f"),
										  QString::null };
	for( int i = 0; !extensions[i].isNull(); ++i ) {
		if( imageFile.endsWith( extensions[i], false ) ) return true;
	}
	return false;
}

namespace {
  RAWImageDecoder rawdecoder;
}
