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
#include <QMetaObject>
#include <KLocale>
#include <KMessageBox>
#include <MainWindow/Window.h>
#include <DB/ImageDB.h>

namespace ImageManager {

#define STR(x) QString::fromUtf8(x)
void ExtractOneVideoFrame::extract(const DB::FileName &fileName, double offset, QObject* receiver, const char* slot)
{
    new ExtractOneVideoFrame(fileName, offset, receiver, slot);
}

ExtractOneVideoFrame::ExtractOneVideoFrame(const DB::FileName &fileName, double offset, QObject *receiver, const char *slot)
{
    m_process = new Utilities::Process(this);
    setupWorkingDirectory();
    m_process->setWorkingDirectory(m_workingDirectory);

    connect( m_process, SIGNAL(finished(int)), this, SLOT(frameFetched()));
    connect( m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(handleError(QProcess::ProcessError)));
    connect( this, SIGNAL(result(QImage)), receiver, slot);

    QStringList arguments;
    arguments << STR("-nosound") << STR("-ss") << QString::number(offset,'f',4) << STR("-vf")
              << STR("screenshot") << STR("-frames") << STR("20") << STR("-vo") << STR("png:z=9") << fileName.absolute();
    //qDebug( "%s %s", qPrintable(MainWindow::FeatureDialog::mplayerBinary()), qPrintable(arguments.join(QString::fromLatin1(" "))));

    m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
}

void ExtractOneVideoFrame::frameFetched()
{
    QImage image(m_workingDirectory + STR("/00000020.png"));
    emit result(image);
    deleteWorkingDirectory();
    deleteLater();
}

void ExtractOneVideoFrame::handleError(QProcess::ProcessError error)
{
    QString message;
    switch (error) {
    case QProcess::FailedToStart: message = i18n("Failed to start"); break;
    case QProcess::Crashed: message = i18n("Crashed"); break;
    case QProcess::Timedout: message = i18n("Timedout"); break;
    case QProcess::ReadError: message = i18n("Read error"); break;
    case QProcess::WriteError: message = i18n("Write error"); break;
    case QProcess::UnknownError: message = i18n("Unknown error"); break;
    }

    KMessageBox::information( MainWindow::Window::theMainWindow(),
            i18n("<p>Error when extracting video thumbnails.<br/>Error was: %1</p>" , message ),
            QString(), QLatin1String("errorWhenRunningQProcessFromExtractOneVideoFrame"));
    emit result(QImage());
    deleteLater();
}

void ExtractOneVideoFrame::setupWorkingDirectory()
{
    const QString tmpPath = STR("%1/KPA-XXXXXX").arg(QDir::tempPath());
    m_workingDirectory = QString::fromUtf8(mkdtemp(tmpPath.toUtf8().data()));
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
// vi:expandtab:tabstop=4 shiftwidth=4:
