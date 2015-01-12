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
#ifndef DATABASEELEMENT_H
#define DATABASEELEMENT_H

#include <qstring.h>
#include <qvariant.h>
namespace Exiv2
{
    class ExifData;
}
class QSqlQuery;

namespace Exif {

class DatabaseElement
{
public:
    virtual ~DatabaseElement() {}
    virtual QString columnName() const = 0;
    /**
     * @brief createString
     * Create a string containing the field name and field data type.
     * E.g.: "Exif_Photo_FNumber_denominator int", "Exif_Photo_FNumber_nominator int"
     * @return a string suitable for usage in a create table query
     */
    virtual QString createString() const = 0;
    /**
     * @brief queryString
     * @return a placeholder string ("?") for the column.
     */
    virtual QString queryString() const = 0;
    /**
     * @brief bindValues bind the ExifData value to the query.
     * @param query the query that the value is bound to.
     * @param the index of the parameter (will be auto-incremented).
     * @param data
     */
    virtual void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data ) const = 0;
    /**
     * @brief bindValues bind a QSql::Out value to the query.
     *
     * @param query the query that the value is bound to.
     */
    virtual void bindValues( QSqlQuery* query, int& counter) = 0;
    /**
     * @brief value the bound value of the query bound with bindValues(QSqlQuery*)
     * @return The bound value, or an empty QVariant if no bindValues was never called.
     */
    QVariant value() const;
    void setValue( QVariant val );
protected:
    DatabaseElement();
private:
    QVariant m_value;
};

class StringExifElement :public DatabaseElement
{
public:
    explicit StringExifElement( const char* tag );
    virtual QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data ) const override;
    virtual void bindValues( QSqlQuery* query, int& counter) override;

private:
    const char* m_tag;
};

class IntExifElement :public DatabaseElement
{
public:
    explicit IntExifElement( const char* tag );
    virtual QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data ) const override;
    virtual void bindValues( QSqlQuery* query, int& counter) override;

private:
    const char* m_tag;
};


/**
 * @brief The RationalExifElement class
 * This has support for the exif rational type.
 *
 * Currently, only simple (one component) rationals and
 * the 3-component rationals used for GPS data (hour-minute-second)
 * are supported.
 */
class RationalExifElement :public DatabaseElement
{
public:
    explicit RationalExifElement( const char* tag );
    virtual QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data ) const override;
    virtual void bindValues( QSqlQuery* query, int& counter) override;

private:
    const char* m_tag;
};


}



#endif /* DATABASEELEMENT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
