#include "AttributeEscaping.h"

#include "CompressFileInfo.h"

#include <QRegularExpression>

QString DB::unescapeAttributeName(const QString &str, int fileVersion)
{
    static bool s_hashUsesCompressedFormat = useCompressedFileFormat();
    static QHash<QString, QString> s_cache;
    static int s_cacheVersion = -1;

    if (s_hashUsesCompressedFormat != useCompressedFileFormat() || s_cacheVersion != fileVersion) {
        s_cache.clear();
        s_cacheVersion = fileVersion;
        s_hashUsesCompressedFormat = useCompressedFileFormat();
    }

    if (s_cache.contains(str)) {
        return s_cache.value(str);
    }

    // If we use the "compressed" file format, we have to do some un-escaping, because the category
    // names have been used as XML attributes and have been escaped before saving.

    // Up to db v10, some Latin-1-only compatible escaping has been done using regular expressions
    // for the "compressed" format. Also, there was some space-underscore substitution for the
    // "readable" format. We need this when reading a pre-v11 db, so we still provide this algorithm
    // here:

    if (fileVersion <= 10) {
        auto unEscaped = str;

        if (!useCompressedFileFormat()) {
            unEscaped.replace(QStringLiteral("_"), QStringLiteral(" "));

        } else {
            QRegularExpression rx(QStringLiteral("(_.)([0-9A-F]{2})"));
            int pos = 0;

            // FIXME: KF6 port: Please review if this still does the same as the QRegExp stuff did
            //
            // Well, it looks like this broke long time ago anyway, because e.g. "Schlagwörter"
            // will be escaped to "Schlagw_.FFFFFFF6rter" by the old escaping algorithm. I suppose
            // this was "Schlagw_.F6rter" at some point in the past, because the above regexp
            // searches for "_." followed by two chars.
            // Anyway, it doesn't matter, because the unescaped strings aren't actually used when
            // reading a database; instead, the escaped category names are compared to what is found
            // and the values are mapped, and the unescaped value is used.
            //
            // Also, with the new db v11 escaping and unescaping, this is obsolete anyway.

            if (useCompressedFileFormat()) {
                auto match = rx.match(unEscaped);
                while (match.hasMatch()) {
                    QString before = match.captured(1) + match.captured(2);
                    QString after = QString::fromLatin1(
                        QByteArray::fromHex(match.captured(2).toLocal8Bit()));
                    unEscaped.replace(pos, before.length(), after);
                    pos += after.length();
                    match = rx.match(unEscaped, pos);
                }
            }
        }

        s_cache.insert(str, unEscaped);
        return unEscaped;
    }

    // Beginning from db v11, we use a modified percent encoding provided by QByteArray that will
    // work for all input strings and covers the whole Unicode range. We only do this for the
    // "compressed" format. For the "readble" format, the string is used as-is.
    if (!useCompressedFileFormat()) {
        return str;
    } else {
        auto unEscaped = str.mid(1); // Here we strip the ":" we added in DB::FileWriter::escape
        unEscaped = QString::fromUtf8(QByteArray::fromPercentEncoding(unEscaped.toUtf8(), '_'));
        s_cache.insert(str, unEscaped);
        return unEscaped;
    }
}

QString DB::escapeAttributeName(const QString &str, int fileVersion)
{
    static bool s_hashUsesCompressedFormat = useCompressedFileFormat();
    static QHash<QString, QString> s_cache;
    static int s_cacheVersion = -1;

    if (s_hashUsesCompressedFormat != useCompressedFileFormat() || s_cacheVersion != fileVersion) {
        s_cache.clear();
        s_cacheVersion = fileVersion;
        s_hashUsesCompressedFormat = useCompressedFileFormat();
    }

    if (s_cache.contains(str)) {
        return s_cache.value(str);
    }

    // Up to db v10, some Latin-1-only compatible escaping has been done using regular expressions
    // for the "compressed" format. Also, there was some space-underscore substitution for the
    // "readable" format. We need this when reading a pre-v11 db, so we still provide this algorithm
    // here:

    if (fileVersion <= 10) {
        QString escaped;

        if (!useCompressedFileFormat()) {
            escaped = str;
            escaped.replace(QStringLiteral(" "), QStringLiteral("_"));

        } else {
            if (useCompressedFileFormat()) {
                static const QRegularExpression rx(QStringLiteral("([^a-zA-Z0-9:_])"));
                QString tmp(str);
                while (true) {
                    const auto match = rx.match(tmp);
                    if (match.hasMatch()) {
                        escaped += tmp.left(match.capturedStart())
                            + QString::asprintf("_.%0X", match.captured().data()->toLatin1());
                        tmp = tmp.mid(match.capturedStart() + match.capturedLength(), -1);
                    } else {
                        escaped += tmp;
                        break;
                    }
                }
            }
        }

        s_cache.insert(str, escaped);
        return escaped;
    }

    if (!useCompressedFileFormat()) {
        // If we use the "readable" format, we don't need escaping:
        return str;

    } else {
        // If we use the "compressed" file format, we have to do some escaping, because the category
        // names are used as XML attributes and we can't use all characters for them. Beginning from
        // db v11, we use a modified percent encoding provided by QByteArray that will work for all
        // input strings and covers the whole Unicode range:

        // The first character of an XML attribute must be a NameStartChar. That is a-z,
        // A-Z, ":" or "_". From the second position on, also 0-9, "." and "-" are allowed
        // (cf. https://www.w3.org/TR/xml/ for the full specification).
        //
        // To keep it simple, we escape everything that is not a small or capital letter, and use
        // the underscore as our escaping char. This way, nothing can go wrong, no matter what the
        // given string contains, and at which position.

        // By default, QByteArray::toPercentEncoding encodes everything that is not a-z, A-Z, 0-9,
        // "-", ".", "_" or "~". We thus have to add some more chars to include:
        static const QByteArray s_escapeIncludes = QStringLiteral("-._~0123456789").toUtf8();

        const auto escaped = QString::fromUtf8(
            str.toUtf8().toPercentEncoding(QByteArray(), s_escapeIncludes, '_'));
        s_cache.insert(str, escaped);

        // We always start with a ":", so that we can't collide with our internal attribute names
        return QStringLiteral(":%1").arg(escaped);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
