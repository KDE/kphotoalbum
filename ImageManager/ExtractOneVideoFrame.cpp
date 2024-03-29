// SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ExtractOneVideoFrame.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/FeatureDialog.h>
#include <MainWindow/TokenEditor.h>
#include <MainWindow/Window.h>
#include <Utilities/Process.h>
#include <kpabase/Logging.h>
#include <kpabase/StringSet.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <QDir>
#include <QTemporaryDir>

namespace ImageManager
{
QString ExtractOneVideoFrame::s_tokenForShortVideos;

#define STR(x) QString::fromUtf8(x)
void ExtractOneVideoFrame::extract(const DB::FileName &fileName, double offset, QObject *receiver, const char *slot)
{
    if (MainWindow::FeatureDialog::hasVideoThumbnailer())
        new ExtractOneVideoFrame(fileName, offset, receiver, slot);
}

void ExtractOneVideoFrame::processFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status == QProcess::ExitStatus::NormalExit && exitCode == 0) {
        frameFetched();
    } else {
        handleError(m_process->error());
    }
}

ExtractOneVideoFrame::ExtractOneVideoFrame(const DB::FileName &fileName, double offset, QObject *receiver, const char *slot)
{
    m_fileName = fileName;
    const QString tmpPath = STR("%1/KPA-XXXXXX").arg(QDir::tempPath());
    m_workingDirectory = new QTemporaryDir(tmpPath);
    if (!m_workingDirectory->isValid())
        qCWarning(ImageManagerLog) << "Failed to create temporary directory!";
    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(m_workingDirectory->path());

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ExtractOneVideoFrame::processFinished);
    connect(this, SIGNAL(result(QImage)), receiver, slot);

    Q_ASSERT(MainWindow::FeatureDialog::hasVideoThumbnailer());
    QStringList arguments;
    // analyzeduration is for videos where the videostream starts later than the sound
    arguments << STR("-ss") << QString::number(offset, 'f', 4) << STR("-analyzeduration")
              << STR("200M") << STR("-i") << fileName.absolute() << STR("-vf") << STR("thumbnail")
              << STR("-vframes") << STR("20") << m_workingDirectory->filePath(STR("000000%02d.png"));

    qCDebug(ImageManagerLog, "%s %s", qPrintable(MainWindow::FeatureDialog::ffmpegBinary()), qPrintable(arguments.join(QString::fromLatin1(" "))));

    m_process->start(MainWindow::FeatureDialog::ffmpegBinary(), arguments);
}

void ExtractOneVideoFrame::frameFetched()
{
    if (!QFile::exists(m_workingDirectory->filePath(STR("00000020.png"))))
        markShortVideo(m_fileName);

    QString name;
    for (int i = 20; i > 0; --i) {
        name = m_workingDirectory->filePath(STR("000000%1.png").arg(i, 2, 10, QChar::fromLatin1('0')));
        if (QFile::exists(name)) {
            qCDebug(ImageManagerLog) << "Using video frame " << i;
            break;
        }
    }

    QImage image(name);
    Q_EMIT result(image);
    delete m_workingDirectory;
    deleteLater();
}

void ExtractOneVideoFrame::handleError(QProcess::ProcessError error)
{
    QString message;
    switch (error) {
    case QProcess::FailedToStart:
        message = i18n("Failed to start");
        break;
    case QProcess::Crashed:
        message = i18n("Crashed");
        break;
    case QProcess::Timedout:
        message = i18n("Timedout");
        break;
    case QProcess::ReadError:
        message = i18n("Read error");
        break;
    case QProcess::WriteError:
        message = i18n("Write error");
        break;
    case QProcess::UnknownError:
        message = i18n("Unknown error");
        break;
    }

    KMessageBox::information(MainWindow::Window::theMainWindow(),
                             i18n("<p>Error when extracting video thumbnails.<br/>Error was: %1</p>", message),
                             QString(), QLatin1String("errorWhenRunningQProcessFromExtractOneVideoFrame"));
    Q_EMIT result(QImage());
    deleteLater();
}

void ExtractOneVideoFrame::markShortVideo(const DB::FileName &fileName)
{
    if (s_tokenForShortVideos.isNull()) {
        const auto tokensInUse = MainWindow::TokenEditor::tokensInUse();
        Utilities::StringSet usedTokens(tokensInUse.begin(), tokensInUse.end());
        for (int ch = 'A'; ch <= 'Z'; ++ch) {
            QString token = QChar::fromLatin1((char)ch);
            if (!usedTokens.contains(token)) {
                s_tokenForShortVideos = token;
                break;
            }
        }

        if (s_tokenForShortVideos.isNull()) {
            // Hmmm, no free token. OK lets just skip setting tokens.
            return;
        }
        KMessageBox::information(MainWindow::Window::theMainWindow(),
                                 i18n("Unable to extract video thumbnails from some files. "
                                      "Either the file is damaged in some way, or the video is ultra short. "
                                      "For your convenience, the token '%1' "
                                      "has been set on those videos.\n\n"
                                      "(You might need to wait till the video extraction led in your status bar has stopped blinking, "
                                      "to see all affected videos.)",
                                      s_tokenForShortVideos));
    }

    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
    DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
    info->addCategoryInfo(tokensCategory->name(), s_tokenForShortVideos);
    MainWindow::DirtyIndicator::markDirty();
}

} // namespace ImageManager
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ExtractOneVideoFrame.cpp"
