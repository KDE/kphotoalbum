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

#ifndef IMPORTEXPORT_IMPORT_H
#define IMPORTEXPORT_IMPORT_H

#include <QObject>
#include <QUrl>

class QTemporaryFile;

class KJob;
namespace KIO {class Job; }

namespace ImportExport
{

class Import :public QObject
{
    Q_OBJECT

public:
    static void imageImport();
    static void imageImport( const QUrl &url );

private slots:
    void downloadKimJobCompleted( KJob* );
    void data( KIO::Job*, const QByteArray& );


private:
    void exec(const QString& fileName );
    void downloadUrl( const QUrl &url );

private:
    Import();
    ~Import() override;
    QTemporaryFile* m_tmp;
    QUrl m_kimFileUrl;
};

}

#endif // IMPORTEXPORT_IMPORT_H

// vi:expandtab:tabstop=4 shiftwidth=4:
