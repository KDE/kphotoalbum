// SPDX-FileCopyrightText: 2012 - 2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-FileCopyrightText: 2025 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "VideoMetaDataExtractor.h"

#include <MainWindow/FeatureDialog.h>
#include <Utilities/Process.h>
#include <kpabase/Logging.h>

#include <QDir>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoMetaDataExtractor::VideoMetaDataExtractor(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
{
}

void ImageManager::VideoMetaDataExtractor::extract(const DB::FileName &fileName)
{
    m_fileName = fileName;
    if (m_process) {
        disconnect(m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));
        m_process->kill();
        delete m_process;
        m_process = nullptr;
    }

    if (!MainWindow::FeatureDialog::hasVideoProber()) {
        Q_EMIT unableToDetermineCreationTime();
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

void ImageManager::VideoMetaDataExtractor::processEnded()
{
    if (!m_process->stdErr().isEmpty())
        qCDebug(ImageManagerLog) << m_process->stdErr();

    const QStringList list = m_process->stdOut().split(QChar::fromLatin1('\n'), Qt::SkipEmptyParts);

    bool creationTimeOK = false;
    bool lengthOK = false;

    // The output looks similar to this (from demo/movie.avi):
    // duration=4.533288
    // TAG:creation_time=2006-10-29 14:33:55
    for (const auto& line : list) {
        const QStringList fields = line.split(QChar::fromLatin1('='));

        if (fields.count() == 2) {
            if (fields[0] == QString::fromLatin1("duration")) {
                const QString lenStr = fields[1].trimmed();
                bool ok = false;
                const double length = lenStr.toDouble(&ok);

                if (ok && length > 0) {
                    lengthOK = true;
                    Q_EMIT lengthFound(int(length));
                } else {
                    qCWarning(ImageManagerLog) << "Unable to parse length from"
                                               << lenStr
                                               << "in line:\n"
                                               << line;
                }
            } else if (fields[0] == QString::fromLatin1("TAG:creation_time")) {
                const QDateTime dateTime = QDateTime::fromString(fields[1], Qt::ISODate);

                if (dateTime.isValid()) {
                    creationTimeOK = true;
                    Q_EMIT creationTimeFound(dateTime);
                }
            } else {
                qCDebug(ImageManagerLog) << "Ignoring line:\n"
                                         << line;
            }
        } else {
            qCWarning(ImageManagerLog) << "Expected 2 fields but found"
                                       << fields.count()
                                       << "in line:\n"
                                       << line;
        }
    }

    if (!creationTimeOK) {
        qCWarning(ImageManagerLog) << "Unable to parse video creation time for"
                                   << m_fileName.relative()
                                   << "from ffprobe output:\n"
                                   << m_process->stdOut();
        Q_EMIT unableToDetermineCreationTime();
    }

    if (!lengthOK) {
        qCWarning(ImageManagerLog) << "Unable to parse video length for"
                                   << m_fileName.relative()
                                   << "from ffprobe output:\n"
                                   << m_process->stdOut();
        Q_EMIT unableToDetermineLength();
    }

    m_process->deleteLater();
    m_process = nullptr;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoMetaDataExtractor.cpp"
