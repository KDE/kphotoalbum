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
#include "DatabaseElement.h"
#include "Logging.h"

#include <QVariant>
#include <exiv2/exif.hpp>

static QString replaceDotWithUnderscore( const char* cstr )
{
    QString str( QString::fromLatin1( cstr ) );
    return str.replace( QString::fromLatin1( "." ), QString::fromLatin1( "_" ) );
}

Exif::DatabaseElement::DatabaseElement()
    : m_value()
{
}

QVariant Exif::DatabaseElement::value() const
{
    return m_value;
}
void Exif::DatabaseElement::setValue( QVariant val )
{
    m_value = val;
}

Exif::StringExifElement::StringExifElement( const char* tag )
    : m_tag( tag )
{
}

QString Exif::StringExifElement::columnName() const
{
    return replaceDotWithUnderscore( m_tag );
}

QString Exif::StringExifElement::createString() const
{
    return QString::fromLatin1( "%1 string" ).arg( replaceDotWithUnderscore( m_tag ) );
}


QString Exif::StringExifElement::queryString() const
{
    return QString::fromLatin1( "?" );
}


QVariant Exif::StringExifElement::valueFromExif(Exiv2::ExifData &data) const
{
    return QVariant{ QLatin1String(data[m_tag].toString().c_str() ) };
}


Exif::IntExifElement::IntExifElement( const char* tag )
    : m_tag( tag )
{
}

QString Exif::IntExifElement::columnName() const
{
    return replaceDotWithUnderscore( m_tag );
}

QString Exif::IntExifElement::createString() const
{
    return QString::fromLatin1( "%1 int" ).arg( replaceDotWithUnderscore( m_tag ) );
}


QString Exif::IntExifElement::queryString() const
{
    return QString::fromLatin1( "?" );
}


QVariant Exif::IntExifElement::valueFromExif(Exiv2::ExifData &data) const
{
    if (data[m_tag].count() > 0)
        return QVariant{ (int) data[m_tag].toLong() };
    else
        return QVariant{ (int) 0 };
}


Exif::RationalExifElement::RationalExifElement( const char* tag )
    : m_tag( tag )
{
}

QString Exif::RationalExifElement::columnName() const
{
    return replaceDotWithUnderscore( m_tag );
}

QString Exif::RationalExifElement::createString() const
{
    return QString::fromLatin1( "%1 float" ).arg( replaceDotWithUnderscore( m_tag ) );
}

QString Exif::RationalExifElement::queryString() const
{
    return QString::fromLatin1( "?" );
}


QVariant Exif::RationalExifElement::valueFromExif(Exiv2::ExifData &data) const
{
    double value;
    Exiv2::Exifdatum &tagDatum = data[m_tag];
    switch ( tagDatum.count() )
    {
        case 0: // empty
            value = -1.0;
            break;
        case 1: // "normal" rational
            value = 1.0 * tagDatum.toRational().first / tagDatum.toRational().second;
            break;
        case 3: // GPS lat/lon data:
        {
            value = 0.0;
            double divisor = 1.0;
            // degree / minute / second:
            for (int i=0 ; i < 3 ; i++ )
            {
                double nom = tagDatum.toRational(i).first;
                double denom = tagDatum.toRational(i).second;
                if ( denom != 0 )
                    value += (nom / denom)/ divisor;
                divisor *= 60.0;
            }
        }
            break;
        default:
            // FIXME: there are at least the following other rational types:
            // whitepoints -> 2 components
            // YCbCrCoefficients -> 3 components (Coefficients for transformation from RGB to YCbCr image data. )
            // chromaticities -> 6 components
            qCWarning(ExifLog) << "Exif rational data with " << tagDatum.count() << " components is not handled, yet!";
            return QVariant{};
    }
    return QVariant{value};
}

Exif::LensExifElement::LensExifElement()
    : m_tag("Exif.Photo.LensModel")
{
}

QString Exif::LensExifElement::columnName() const
{
    return replaceDotWithUnderscore( m_tag );
}

QString Exif::LensExifElement::createString() const
{
    return QString::fromLatin1( "%1 string" ).arg( replaceDotWithUnderscore( m_tag ) );
}


QString Exif::LensExifElement::queryString() const
{
    return QString::fromLatin1( "?" );
}


QVariant Exif::LensExifElement::valueFromExif(Exiv2::ExifData &data) const
{
    QString value;
    bool canonHack = false;
    for (Exiv2::ExifData::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        const QString datum = QString::fromLatin1(it->key().c_str());

        // Exif.Photo.LensModel [Ascii]
        // Exif.Canon.LensModel [Ascii]
        // Exif.OlympusEq.LensModel [Ascii]
        if (datum.endsWith(QString::fromLatin1(".LensModel")))
        {
            qCDebug(ExifLog) << datum << ": " << it->toString().c_str();
            canonHack = false;
            value = QString::fromUtf8(it->toString().c_str());
            // we can break here since Exif.Photo.LensModel should be bound first
            break;
        }

        // Exif.NikonLd3.LensIDNumber [Byte]
        // on Nikon cameras, this seems to provide better results than .Lens and .LensType
        // (i.e. it includes the lens manufacturer).
        if (datum.endsWith(QString::fromLatin1(".LensIDNumber")))
        {
            // ExifDatum::print() returns the interpreted value
            qCDebug(ExifLog) << datum << ": " << it->print(&data).c_str();
            canonHack = false;
            value = QString::fromUtf8(it->print(&data).c_str());
            continue;
        }

        // Exif.Nikon3.LensType [Byte]
        // Exif.OlympusEq.LensType [Byte]
        // Exif.Panasonic.LensType [Ascii]
        // Exif.Pentax.LensType [Byte]
        // Exif.Samsung2.LensType [Short]
        if (datum.endsWith(QString::fromLatin1(".LensType")))
        {
            // ExifDatum::print() returns the interpreted value
            qCDebug(ExifLog) << datum << ": " << it->print(&data).c_str();
            // make sure this cannot overwrite LensIDNumber
            if (value.isEmpty())
            {
                canonHack = (datum == QString::fromLatin1("Exif.CanonCs.LensType"));
                value = QString::fromUtf8(it->print(&data).c_str());
            }
        }
    }

    // some canon lenses have a dummy value as LensType:
    if (canonHack && value == QString::fromLatin1("(65535)"))
    {
        value = QString::fromLatin1("Canon generic");
        const auto datum = data.findKey(Exiv2::ExifKey("Exif.CanonCs.Lens"));
        if (datum != data.end())
        {
            value += QString::fromLatin1(" ");
            value += QString::fromUtf8(datum->print(&data).c_str());
        }

    }
    qCDebug(ExifLog) << "final lens value " << value;
    return QVariant{ value };
}

// vi:expandtab:tabstop=4 shiftwidth=4:
