/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KIMFILEREADER_H
#define KIMFILEREADER_H
#include <QPixmap>
#include <QString>
class KArchiveDirectory;
class KZip;

namespace ImportExport
{

class KimFileReader
{
public:
    KimFileReader();
    ~KimFileReader();
    bool open(const QString &fileName);
    QByteArray indexXML();
    QPixmap loadThumbnail(QString fileName);
    QByteArray loadImage(const QString &fileName);

private:
    QString m_fileName;
    KZip *m_zip;
    const KArchiveDirectory *m_dir;
};

}

#endif /* KIMFILEREADER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
