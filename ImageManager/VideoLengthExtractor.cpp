/* Copyright 2012-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "VideoLengthExtractor.h"
#include "Logging.h"

#include <MainWindow/FeatureDialog.h>
#include <Utilities/Process.h>

#include <QDir>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoLengthExtractor::VideoLengthExtractor(QObject *parent) :
    QObject(parent), m_process(nullptr)
{
}

void ImageManager::VideoLengthExtractor::extract(const DB::FileName &fileName)
{
    m_fileName = fileName;
    if ( m_process ) {
        disconnect( m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));
        m_process->kill();
        delete m_process;
        m_process = nullptr;
    }

    if (!MainWindow::FeatureDialog::hasVideoThumbnailer()) {
        emit unableToDetermineLength();
        return;
    }

    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(QDir::tempPath());
    connect( m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));

    if (MainWindow::FeatureDialog::ffmpegBinary().isEmpty())
    {
        QStringList arguments;
        arguments << STR("-identify") << STR("-frames") << STR("0") << STR("-vc") << STR("null")
                  << STR("-vo") << STR("null") << STR("-ao") << STR("null") <<  fileName.absolute();

        m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
    } else {
        QStringList arguments;
        // Just look at the length of the container. Some videos have streams without duration entry
        arguments << STR("-v") << STR("0") << STR("-show_entries") << STR("format=duration")
                  << STR("-of") << STR("default=noprint_wrappers=1:nokey=1")
                  <<  fileName.absolute();

        qCDebug(ImageManagerLog, "%s %s", qPrintable(MainWindow::FeatureDialog::ffprobeBinary()), qPrintable(arguments.join(QString::fromLatin1(" "))));
        m_process->start(MainWindow::FeatureDialog::ffprobeBinary(), arguments);
    }
}

void ImageManager::VideoLengthExtractor::processEnded()
{
    if ( !m_process->stdErr().isEmpty() )
        qCDebug(ImageManagerLog) << m_process->stdErr();

    QString lenStr;
    if (MainWindow::FeatureDialog::ffmpegBinary().isEmpty())
    {
        QStringList list = m_process->stdOut().split(QChar::fromLatin1('\n'));
        list = list.filter(STR("ID_LENGTH="));
        if ( list.count() == 0 ) {
            qCWarning(ImageManagerLog) << "Unable to find ID_LENGTH in output from MPlayer for file " << m_fileName.absolute() << "\n"
                       << "Output was:\n"
                       << m_process->stdOut();
            emit unableToDetermineLength();
            return;
        }

        const QString match = list[0];
        const QRegExp regexp(STR("ID_LENGTH=([0-9.]+)"));
        if (!regexp.exactMatch(match))
        {
            qCWarning(ImageManagerLog) << STR("Unable to match regexp for string: %1 (for file %2)").arg(match).arg(m_fileName.absolute());
            emit unableToDetermineLength();
            return;
        }

        lenStr = regexp.cap(1);
    } else {
        QStringList list = m_process->stdOut().split(QChar::fromLatin1('\n'));
        // ffprobe -v 0 just prints one line, except if panicking
        if ( list.count() < 1 ) {
            qCWarning(ImageManagerLog) << "Unable to parse video length from ffprobe output!"
                       << "Output was:\n"
                       << m_process->stdOut();
            emit unableToDetermineLength();
            return;
        }
        lenStr = list[0].trimmed();
    }

    bool ok = false;
    const double length = lenStr.toDouble(&ok);
    if ( !ok ) {
        qCWarning(ImageManagerLog) << STR("Unable to convert string \"%1\"to double (for file %2)").arg(lenStr).arg(m_fileName.absolute());
        emit unableToDetermineLength();
        return;
    }

    if ( length == 0 ) {
        qCWarning(ImageManagerLog) << "video length returned was 0 for file " << m_fileName.absolute();
        emit unableToDetermineLength();
        return;
    }

    emit lengthFound(length);
    m_process->deleteLater();
    m_process = nullptr;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
