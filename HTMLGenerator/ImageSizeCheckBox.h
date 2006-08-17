#ifndef HTMLGENERATOR_IMAGESIZECHECKBOX_H
#define HTMLGENERATOR_IMAGESIZECHECKBOX_H

#include <qcheckbox.h>

namespace HTMLGenerator
{
class ImageSizeCheckBox :public QCheckBox {

public:
    ImageSizeCheckBox( int width, int height, QWidget* parent )
        :QCheckBox( QString::fromLatin1("%1x%2").arg(width).arg(height), parent ),
         _width( width ), _height( height )
        {
        }

    ImageSizeCheckBox( const QString& text, QWidget* parent )
        :QCheckBox( text, parent ), _width( -1 ), _height( -1 )
        {
        }

    int width() const {
        return _width;
    }
    int height() const {
        return _height;
    }
    QString text( bool withOutSpaces ) const {
        return text( _width, _height, withOutSpaces );
    }
    static QString text( int width, int height, bool withOutSpaces ) {
        if ( width == -1 )
            if ( withOutSpaces )
                return QString::fromLatin1("fullsize");
            else
                return QString::fromLatin1("full size");

        else
            return QString::fromLatin1("%1x%2").arg(width).arg(height);
    }

    bool operator<( const ImageSizeCheckBox& other )
    {
        return _width < other.width();
    }


private:
    int _width;
    int _height;
};

}


#endif /* HTMLGENERATOR_IMAGESIZECHECKBOX_H */

