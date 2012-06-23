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

#include "ExtractOneVideoFrame.h"
#include <Utilities/Process.h>
#include <MainWindow/FeatureDialog.h>
#include <QDir>
#include <cstdlib>

namespace ImageManager {

ExtractOneVideoFrame::ExtractOneVideoFrame(QObject *parent) :
    QObject(parent)
{
    m_process = new Utilities::Process(this);
    setupWorkingDirectory();
    m_process->setWorkingDirectory(m_workingDirectory);
    // PENDING HOW ABOUT ERROR HANDLING?
    connect( m_process, SIGNAL(finished(int)), this, SLOT(frameFetched()));
}

#define STR(x) QString::fromUtf8(x)
void ExtractOneVideoFrame::extract(const DB::FileName &fileName, int offset)
{
    QStringList arguments;
    arguments << STR("-nosound") << STR("-ss") << QString::number(offset,'g',2) << STR("-vf")
              << STR("screenshot") << STR("-frames") << STR("1") << STR("-vo") << STR("png:z=9") << fileName.absolute();

    m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
}

void ExtractOneVideoFrame::frameFetched()
{
    QImage image(m_workingDirectory + STR("/00000001.png"));
    Q_ASSERT(!image.isNull());
    emit frameFetched(image);
    deleteWorkingDirectory();
}

void ExtractOneVideoFrame::setupWorkingDirectory()
{
    const QString tmpPath = STR("%1/KPA-XXXXXX").arg(QDir::tempPath());
    char* cTmpPath = tmpPath.toUtf8().data();
    m_workingDirectory = QString::fromUtf8(mkdtemp(cTmpPath));
}

void ExtractOneVideoFrame::deleteWorkingDirectory()
{
    QDir dir(m_workingDirectory);
    QStringList files = dir.entryList(QDir::Files);
    Q_FOREACH( const QString& file, files )
        dir.remove(file);

    dir.rmdir(m_workingDirectory);
}

} // namespace ImageManager
