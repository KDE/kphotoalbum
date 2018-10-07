/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "FastJpeg.h"

#include "Logging.h"
#include "JpeglibWithFix.h"

#include "DB/FileName.h"

#include <QFileInfo>
#include <QImageReader>
#include <QVector>

#include <csetjmp>
extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
}

struct myjpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

extern "C"
{
    static void myjpeg_error_exit(j_common_ptr cinfo)
    {
        auto* myerr = (myjpeg_error_mgr*) cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        //kWarning() << buffer;
        longjmp(myerr->setjmp_buffer, 1);
    }
}

namespace Utilities
{
    static bool loadJPEGInternal(QImage *img, FILE* inputFile, QSize* fullSize, int dim, const char *membuf, size_t membuf_size );
}

bool Utilities::loadJPEG(QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim, char *membuf, size_t membufSize)
{
    bool ok;
    struct stat statbuf;
    if ( stat( QFile::encodeName(imageFile.absolute()).constData(), &statbuf ) == -1 )
        return false;
    if ( ! membuf || statbuf.st_size > (int) membufSize ) {
        qCDebug(UtilitiesLog) << "loadJPEG (slow path) " << imageFile.relative() << " " << statbuf.st_size << " " << membufSize;
        FILE* inputFile=fopen( QFile::encodeName(imageFile.absolute()).constData(), "rb");
        if(!inputFile)
            return false;
        ok = loadJPEGInternal( img, inputFile, fullSize, dim, NULL, 0 );
        fclose(inputFile);
    } else {
        // May be more than is needed, but less likely to fragment memory this
        // way.
        int inputFD = open( QFile::encodeName(imageFile.absolute()).constData(), O_RDONLY );
        if ( inputFD == -1 ) {
            return false;
        }
        unsigned bytesLeft = statbuf.st_size;
        unsigned offset = 0;
        while ( bytesLeft > 0 ) {
            int bytes = read(inputFD, membuf + offset, bytesLeft);
            if (bytes <= 0) {
                (void) close(inputFD);
                return false;
            }
            offset += bytes;
            bytesLeft -= bytes;
        }
        ok = loadJPEGInternal( img, NULL, fullSize, dim, membuf, statbuf.st_size );
        (void) close(inputFD);
    }
    return ok;
}

bool Utilities::loadJPEG(QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim)
{
    return loadJPEG( img, imageFile, fullSize, dim, NULL, 0 );
}

bool Utilities::loadJPEG(QImage *img, const QByteArray &data, QSize* fullSize, int dim)
{
    return loadJPEGInternal(img, nullptr, fullSize, dim, data.data(), data.size());
}

bool Utilities::loadJPEGInternal(QImage *img, FILE* inputFile, QSize* fullSize, int dim, const char *membuf, size_t membuf_size )
{
    struct jpeg_decompress_struct    cinfo;
    struct myjpeg_error_mgr jerr;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err             = jpeg_std_error(&jerr);
    cinfo.err->error_exit = myjpeg_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    if (inputFile)
        jpeg_stdio_src(&cinfo, inputFile);
    else
        jpeg_mem_src(&cinfo, (unsigned char *) membuf, membuf_size);
    jpeg_read_header(&cinfo, TRUE);
    *fullSize = QSize( cinfo.image_width, cinfo.image_height );

    int imgSize = qMax(cinfo.image_width, cinfo.image_height);

    //libjpeg supports a sort of scale-while-decoding which speeds up decoding
    int scale=1;
    if (dim != -1) {
        while(dim*scale*2<=imgSize) {
            scale*=2;
        }
        if(scale>8) scale=8;
    }

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;

    // Create QImage
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        *img = QImage(
            cinfo.output_width, cinfo.output_height, QImage::Format_RGB32);
        if (img->isNull())
            return false;
        break;
    case 1: // B&W image
        *img = QImage(
            cinfo.output_width, cinfo.output_height, QImage::Format_Indexed8);
        if (img->isNull())
            return false;
        img->setColorCount(256);
        for (int i=0; i<256; i++)
            img->setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    QVector<uchar*> linesVector;
    linesVector.reserve(img->height());
    for (int i = 0; i < img->height(); ++i)
        linesVector.push_back(img->scanLine(i));
    uchar** lines = linesVector.data();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = img->scanLine(j) + cinfo.output_width*3;
            QRgb *out = reinterpret_cast<QRgb*>( img->scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    /*int newMax = qMax(cinfo.output_width, cinfo.output_height);
      int newx = size_*cinfo.output_width / newMax;
      int newy = size_*cinfo.output_height / newMax;*/

    jpeg_destroy_decompress(&cinfo);

    //image = img.smoothScale(newx,newy);
    return true;
}

bool Utilities::isJPEG( const DB::FileName& fileName )
{
    QString format= QString::fromLocal8Bit( QImageReader::imageFormat( fileName.absolute() ) );
    return format == QString::fromLocal8Bit( "jpeg" );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
