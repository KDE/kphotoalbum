#ifndef ATTRIBUTEESCAPING_H
#define ATTRIBUTEESCAPING_H

#include <QString>

namespace DB
{

/**
 * @brief Unescape a string used as an XML attribute name.
 *
 * @see escape
 *
 * @param str the string to be unescaped
 * @param int the db version we want to unescapeAttributeName from
 * @return the unescaped string
 */
QString unescapeAttributeName(const QString &, int fileVersion);

/**
 * @brief Escape problematic characters in a string that forms an XML attribute name.
 *
 * N.B.: Attribute values do not need to be escaped!
 * @see unescape
 *
 * @param str the string to be escaped
 * @param int the db version we want to escapeAttributeName for
 * @return the escaped string
 */
QString escapeAttributeName(const QString &, int fileVersion);

}

#endif // ATTRIBUTEESCAPING_H

// vi:expandtab:tabstop=4 shiftwidth=4:
