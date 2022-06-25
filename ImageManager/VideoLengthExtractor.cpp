/* SPDX-FileCopyrightText: 2012-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "VideoLengthExtractor.h"

#include <MainWindow/FeatureDialog.h>
#include <Utilities/Process.h>
#include <kpabase/Logging.h>

#include <QDir>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoLengthExtractor::VideoLengthExtractor(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
{
}

void ImageManager::VideoLengthExtractor::extract(const DB::FileName &fileName)
{
    m_fileName = fileName;
    if (m_process) {
        disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));
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
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));

    Q_ASSERT(MainWindow::FeatureDialog::hasVideoProber());
    QStringList arguments;
    // Just look at the length of the container. Some videos have streams without duration entry
    arguments << STR("-v") << STR("0") << STR("-show_entries") << STR("format=duration")
              << STR("-of") << STR("default=noprint_wrappers=1:nokey=1")
              << fileName.absolute();

    qCDebug(ImageManagerLog, "%s %s", qPrintable(MainWindow::FeatureDialog::ffprobeBinary()), qPrintable(arguments.join(QString::fromLatin1(" "))));
    m_process->start(MainWindow::FeatureDialog::ffprobeBinary(), arguments);
}

void ImageManager::VideoLengthExtractor::processEnded()
{
    if (!m_process->stdErr().isEmpty())
        qCDebug(ImageManagerLog) << m_process->stdErr();

    const QStringList list = m_process->stdOut().split(QChar::fromLatin1('\n'));
    // ffprobe -v 0 just prints one line, except if panicking
    if (list.count() < 1) {
        qCWarning(ImageManagerLog) << "Unable to parse video length from ffprobe output!"
                                   << "Output was:\n"
                                   << m_process->stdOut();
        emit unableToDetermineLength();
        return;
    }
    const QString lenStr = list[0].trimmed();

    bool ok = false;
    const double length = lenStr.toDouble(&ok);
    if (!ok) {
        qCWarning(ImageManagerLog) << STR("Unable to convert string \"%1\"to double (for file %2)").arg(lenStr).arg(m_fileName.absolute());
        emit unableToDetermineLength();
        return;
    }

    if (length == 0) {
        qCWarning(ImageManagerLog) << "video length returned was 0 for file " << m_fileName.absolute();
        emit unableToDetermineLength();
        return;
    }

    emit lengthFound(int(length));
    m_process->deleteLater();
    m_process = nullptr;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoLengthExtractor.cpp"
