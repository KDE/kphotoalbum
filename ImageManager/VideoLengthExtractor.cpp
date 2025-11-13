// SPDX-FileCopyrightText: 2012 - 2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-FileCopyrightText: 2025 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

    if (!MainWindow::FeatureDialog::hasVideoProber()) {
        Q_EMIT unableToDetermineLength();
        return;
    }

    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(QDir::tempPath());
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));

    Q_ASSERT(MainWindow::FeatureDialog::hasVideoProber());
    QStringList arguments;
    // Just look at the length of the container. Some videos have streams without duration entry
    arguments << STR("-v") << STR("0") << STR("-show_entries")
              << STR("format_tags=creation_time:format=duration")
              << STR("-of") << STR("default=noprint_wrappers=1:nokey=0")
              << fileName.absolute();

    qCDebug(ImageManagerLog, "%s %s", qPrintable(MainWindow::FeatureDialog::ffprobeBinary()), qPrintable(arguments.join(QString::fromLatin1(" "))));
    m_process->start(MainWindow::FeatureDialog::ffprobeBinary(), arguments);
}

void ImageManager::VideoLengthExtractor::processEnded()
{
    if (!m_process->stdErr().isEmpty())
        qCDebug(ImageManagerLog) << m_process->stdErr();

    const QStringList list = m_process->stdOut().split(QChar::fromLatin1('\n'), Qt::SkipEmptyParts);
    // ffprobe -v 0 just prints two lines, except if panicking
    if (list.count() < 2) {
        qCWarning(ImageManagerLog) << "Unable to parse video length from ffprobe output!"
                                   << "Output was:\n"
                                   << m_process->stdOut();
        Q_EMIT unableToDetermineCreationTime();
        Q_EMIT unableToDetermineLength();
        return;
    }

    // The output looks similar to this (from demo/movie.avi):
    // duration=4.533288
    // TAG:creation_time=2006-10-29 14:33:55
    for (auto line : list) {
        const QStringList fields = line.split(QChar::fromLatin1('='));

        if (fields.count() != 2) {
            qCWarning(ImageManagerLog) << "Unable to parse ffprobe output!"
                                       << "Line was:\n"
                                       << line;
            Q_EMIT unableToDetermineCreationTime();
            Q_EMIT unableToDetermineLength();
            return;
        }

        if (fields[0] == QString::fromLatin1("duration")) {
            const QString lenStr = fields[1].trimmed();

            bool ok = false;
            const double length = lenStr.toDouble(&ok);
            if (!ok) {
                qCWarning(ImageManagerLog) << STR("Unable to convert string \"%1\"to double (for file %2)").arg(lenStr, m_fileName.absolute());
                Q_EMIT unableToDetermineLength();
                return;
            }

            if (length == 0) {
                qCWarning(ImageManagerLog) << "video length returned was 0 for file " << m_fileName.absolute();
                Q_EMIT unableToDetermineLength();
                return;
            }

            Q_EMIT lengthFound(int(length));
        } else if (fields[0] == QString::fromLatin1("TAG:creation_time")) {
            // TAG:creation_time=2006-10-29 14:33:55
            const QDateTime dateTime = QDateTime::fromString(fields[1], Qt::ISODate);

            if (dateTime.isValid()) {
                Q_EMIT creationTimeFound(dateTime);
            } else {
                Q_EMIT unableToDetermineCreationTime();
            }
        }
    }

    m_process->deleteLater();
    m_process = nullptr;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoLengthExtractor.cpp"
