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

#include <kurl.h>
class KJob;
namespace KIO {class Job; }

class KTemporaryFile;

namespace ImportExport
{

class Import :public QObject
{
    Q_OBJECT

public:
    static void imageImport();
    static void imageImport( const KUrl& url );

private slots:
    void downloadKimJobCompleted( KJob* );
    void data( KIO::Job*, const QByteArray& );


private:
    void exec(const QString& fileName );
    void downloadUrl( const KUrl& url );

private:
    Import();
    ~Import();
    KTemporaryFile* m_tmp;
    KUrl m_kimFileUrl;
};

}
