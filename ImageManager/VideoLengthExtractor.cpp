#include "VideoLengthExtractor.h"
#include <Utilities/Process.h>
#include <QDir>
#include <MainWindow/FeatureDialog.h>
#include <KDebug>

#define STR(x) QString::fromUtf8(x)

ImageManager::VideoLengthExtractor::VideoLengthExtractor(QObject *parent) :
    QObject(parent)
{
    m_process = new Utilities::Process(this);
    m_process->setWorkingDirectory(QDir::tempPath());
    connect( m_process, SIGNAL(finished(int)), this, SLOT(processEnded()));
}

void ImageManager::VideoLengthExtractor::extract(const QString &fileName)
{
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
