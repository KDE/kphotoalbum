/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DATABASEELEMENT_H
#define DATABASEELEMENT_H

#include <QString>
#include <QVariant>

namespace Exiv2
{
class ExifData;
}

namespace Exif
{

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
     * @brief exifAsValue interprets the ExifData value and creates a QVariant suitable for QSqlQuery::bindValue.
     * @param data
     * @return The converted value, or an empty QVariant if the necessary data is not available.
     */
    virtual QVariant valueFromExif(Exiv2::ExifData &data) const = 0;
    /**
     * @brief value
     * @see Database::readFields
     * @return The bound value, or an empty QVariant if setValue was never called.
     */
    QVariant value() const;
    void setValue(QVariant val);

protected:
    DatabaseElement();

private:
    QVariant m_value;
};

class StringExifElement : public DatabaseElement
{
public:
    explicit StringExifElement(const char *tag);
    QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    QVariant valueFromExif(Exiv2::ExifData &data) const override;

private:
    const char *m_tag;
};

class IntExifElement : public DatabaseElement
{
public:
    explicit IntExifElement(const char *tag);
    QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    QVariant valueFromExif(Exiv2::ExifData &data) const override;

private:
    const char *m_tag;
};

/**
 * @brief The RationalExifElement class
 * This has support for the exif rational type.
 *
 * Currently, only simple (one component) rationals and
 * the 3-component rationals used for GPS data (hour-minute-second)
 * are supported.
 */
class RationalExifElement : public DatabaseElement
{
public:
    explicit RationalExifElement(const char *tag);
    QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    QVariant valueFromExif(Exiv2::ExifData &data) const override;

private:
    const char *m_tag;
};

/**
 * @brief The LensExifElement class
 * This class provides a wrapper around the Exif.Photo.LensModel exif tag.
 * Besides this tag, there are several other legacy exif tags with similar information.
 * This class transparently falls back to a legacy tag when the LensModel is not available.
 */
class LensExifElement : public DatabaseElement
{
public:
    explicit LensExifElement();
    QString columnName() const override;
    QString createString() const override;
    QString queryString() const override;
    QVariant valueFromExif(Exiv2::ExifData &data) const override;

private:
    const char *m_tag;
};

}

#endif /* DATABASEELEMENT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
