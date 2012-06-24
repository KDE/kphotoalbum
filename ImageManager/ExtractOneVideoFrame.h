/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H
#define IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H

#include <QObject>
#include <DB/FileName.h>
class QImage;

namespace Utilities { class Process; }

namespace ImageManager {

class ExtractOneVideoFrame : public QObject
{
    Q_OBJECT
public:
    static void extract(const DB::FileName& filename, int offset, QObject* receiver, const char* slot);

private slots:
    void frameFetched();

signals:
    void result(const QImage& );

private:
    ExtractOneVideoFrame(const DB::FileName& filename, int offset, QObject* receiver, const char* slot);
    void setupWorkingDirectory();
    void deleteWorkingDirectory();

    QString m_workingDirectory;
    Utilities::Process* m_process;
    QObject* m_receiver;
    const char* m_slot;
};

} // namespace ImageManager

#endif // IMAGEMANAGER_EXTRACTONEVIDEOFRAME_H
