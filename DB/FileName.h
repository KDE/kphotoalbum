/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILENAME_H
#define FILENAME_H

#include <QSet>
#include <QString>
#include <QUrl>
#include <QtCore/qmetatype.h>

namespace DB
{

class FileName
{
public:
    FileName();
    static FileName fromAbsolutePath(const QString &fileName);
    static FileName fromRelativePath(const QString &fileName);
    QString absolute() const;
    QString relative() const;
    bool isNull() const;
    bool operator==(const FileName &other) const;
    bool operator!=(const FileName &other) const;
    bool operator<(const FileName &other) const;
    bool exists() const;

    /**
     * @brief Conversion to absolute local file url.
     */
    explicit operator QUrl() const;

private:
    // During previous profilation it showed that converting between absolute and relative took quite some time,
    // so to avoid that, I store both.
    QString m_relativePath;
    QString m_absoluteFilePath;
    bool m_isNull;
};

uint qHash(const DB::FileName &fileName);
typedef QSet<DB::FileName> FileNameSet;
}

Q_DECLARE_METATYPE(DB::FileName)

#endif // FILENAME_H
// vi:expandtab:tabstop=4 shiftwidth=4:
