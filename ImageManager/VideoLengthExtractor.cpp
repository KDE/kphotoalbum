/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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

#include "VideoLengthExtractor.h"
#include <Utilities/Process.h>
#include <QDir>
#include <MainWindow/FeatureDialog.h>
#include <KDebug>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoLengthExtractor::VideoLengthExtractor(QObject *parent) :
    QObject(parent), m_process(0)
{
}

void ImageManager::VideoLengthExtractor::extract(const QString &fileName)
{
    delete m_process;

    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(QDir::tempPath());
    connect( m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));

    if (MainWindow::FeatureDialog::mplayerBinary().isEmpty()) {
        kWarning() << "mplayer not found. Unable to extract length of video file";
        emit unableToDetermineLength();
        return;
    }

    QStringList arguments;
    arguments << STR("-identify") << STR("-frames") << STR("0") << STR("-vc") << STR("null")
              << STR("-vo") << STR("null") << STR("-ao") << STR("null") <<  fileName;

    m_process->start(MainWindow::FeatureDialog::mplayerBinary(), arguments);
}

void ImageManager::VideoLengthExtractor::processEnded()
{
    QStringList list = m_process->stdout().split(QChar::fromLatin1('\n'));
    list = list.filter(STR("ID_LENGTH="));
    if ( list.count() == 0 ) {
        kWarning() << "Unable to find ID_LENGTH in output from mplayer\n"
                   << "Output was:\n"
                   << m_process->stdout();
        emit unableToDetermineLength();
        return;
    }

    const QString match = list[0];
    const QRegExp regexp(STR("ID_LENGTH=([0-9.]+)"));
    bool ok = regexp.exactMatch(match);
    if ( !ok ) {
        kWarning() << STR("Unable to match regexp for string: %1").arg(match);
        emit unableToDetermineLength();
        return;
    }

    const QString cap = regexp.cap(1);

    const int length = cap.toDouble(&ok);
    if ( !ok ) {
        kWarning() << STR("Unable to convert string \"%s\"to integer").arg(cap);
        emit unableToDetermineLength();
        return;
    }

    if ( length == 0 ) {
        kWarning() << "video length returned was 0";
        emit unableToDetermineLength();
        return;
    }

    emit lengthFound(length);
}
