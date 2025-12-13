// SPDX-FileCopyrightText: 2021 Yuri Chornoivan <yurchor@ukr.net>
// SPDX-FileCopyrightText: 2021-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2021-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoPlayerSelectorDialog.h"
#include "Logging.h"
#include <kpabase/config-kpa-videobackends.h>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLoggingCategory>
#include <QRadioButton>
#include <QVBoxLayout>

namespace Settings
{

VideoPlayerSelectorDialog::VideoPlayerSelectorDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(i18n("<h1>Choose a Video Player</h1>"));
    layout->addWidget(label);

    QString txt = i18n("<html><p>Unfortunately, there is no video player which just works out of the box for everyone.</p>"
                       "<p>KPhotoAlbum therefore comes with three different back-ends, choose the one that works the best for you</p>"

                       "<p><b>VLC</b> seems to be the best supported video player back-end, meaning it can play most video formats. "
                       "It has one drawback though, it requires X11. "
                       "If you use Wayland, then you will find that the videos shows up in a window of their own, and in this case "
                       "VLC is likely not a very good fit for you. If on the other hand you do use X11 then VLC is likely your best choice.</p>"

                       "<p>Traditionally KPhotoAlbum has been using <b>Phonon</b> as the video player back-end. We've unfortunately seen it crash "
                       "on many different video formats - possibly because our system wasn't correctly configured.</p>"

                       "<p><b>QtMultimedia</b> is the newest video player back-end of the three. It is a standard component of Qt. "
                       "Expect it to become the new default, and maybe only back-end.</p>"

                       "<p><b><font color=red>You can at any time change the backend from Settings -> Viewer</font></b></p></html>");
    label = new QLabel(txt);
    label->setWordWrap(true);
    layout->addWidget(label);

    m_qtmm = new QRadioButton(QString::fromUtf8("QtMultimedia"));
    layout->addWidget(m_qtmm);

    bool somethingNotAvailable = false;
    if (availableVideoBackends().testFlag(VideoBackend::VLC)) {
        m_vlc = new QRadioButton(QString::fromUtf8("VLC"));
    } else {
        m_vlc = new QRadioButton(QString::fromUtf8("VLC") + i18n(" (NOT AVAILABLE)"));
        m_vlc->setEnabled(false);
        somethingNotAvailable = true;
    }
    layout->addWidget(m_vlc);

    if (availableVideoBackends().testFlag(VideoBackend::Phonon)) {
        m_phonon = new QRadioButton(QString::fromUtf8("Phonon"));
    } else {
        m_phonon = new QRadioButton(QString::fromUtf8("Phonon") + i18n(" (NOT AVAILABLE)"));
        m_phonon->setEnabled(false);
        somethingNotAvailable = true;
    }
    layout->addWidget(m_phonon);

    auto backend = preferredVideoBackend(SettingsData::instance()->videoBackend());
    QRadioButton *rb = [&] {
        QRadioButton *candidate = nullptr;
        switch (backend) {
        case VideoBackend::QtMultimedia:
            candidate = m_qtmm;
        case VideoBackend::Phonon:
            candidate = m_phonon;
            break;
        case VideoBackend::VLC:
            candidate = m_vlc;
            break;
        case VideoBackend::QtAV: // legacy value
        default:
            Q_UNREACHABLE();
        }
        return candidate;
    }();

    rb->setChecked(true);

    if (somethingNotAvailable) {
        auto label = new QLabel(i18n("<html><h1>Missing back-end</h1>"
                                     "The back-ends which are not available are due to the system configuration at compile time of KPhotoAlbum. "
                                     "If you compiled KPhotoAlbum yourself, then search for the developer packages of the back-ends. "
                                     "If you on the other hand got KPhotoAlbum from your system, then please talk to the maintainer of the package.</html>"));
        label->setWordWrap(true);
        layout->addWidget(label);
    }

    // Only offer OK, as it would be way to much work to handle the user not selecting a backend, when we are about to play videos.
    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(box);
}

VideoBackend VideoPlayerSelectorDialog::backend() const
{
    if (m_qtmm->isChecked())
        return VideoBackend::QtMultimedia;
    else if (m_vlc->isChecked())
        return VideoBackend::VLC;
    else
        return VideoBackend::Phonon;
}

constexpr VideoBackends availableVideoBackends()
{
    VideoBackends availableBackends = VideoBackend::QtMultimedia;
#if LIBVLC_FOUND
    availableBackends |= VideoBackend::VLC;
#endif
#if Phonon4Qt6_FOUND
    availableBackends |= VideoBackend::Phonon;
#endif
    return availableBackends;
}

VideoBackend preferredVideoBackend(const VideoBackend configuredBackend, const VideoBackends exclusions)
{
    if (availableVideoBackends().testFlag(configuredBackend) && !exclusions.testFlag(configuredBackend)) {
        qCDebug(SettingsLog) << "preferredVideoBackend(): configured backend is viable:" << configuredBackend;
        return configuredBackend;
    }

    // change backend priority here:
    for (const VideoBackend candidate : { VideoBackend::QtMultimedia, VideoBackend::Phonon, VideoBackend::VLC }) {
        if (availableVideoBackends().testFlag(candidate) && !exclusions.testFlag(candidate)) {
            qCDebug(SettingsLog) << "preferredVideoBackend(): backend is viable:" << candidate;
            return candidate;
        }
        qCDebug(SettingsLog) << "preferredVideoBackend(): backend is not viable:" << candidate;
    }
    qCDebug(SettingsLog) << "preferredVideoBackend(): no backend is viable.";
    return VideoBackend::NotConfigured;
}

QString localizedEnumName(const VideoBackend backend)
{
    switch (backend) {
    case Settings::VideoBackend::QtMultimedia:
        return i18nc("A friendly name for the video backend", "QtMultimedia video backend");
        break;
    case VideoBackend::NotConfigured:
        return i18nc("A friendly name for the video backend", "Unconfigured video backend");
    case VideoBackend::Phonon:
        return i18nc("A friendly name for the video backend", "Phonon video backend");
    case VideoBackend::QtAV: // legacy value; no longer used actively
        return i18nc("A friendly name for the video backend", "QtAV video backend");
    case VideoBackend::VLC:
        return i18nc("A friendly name for the video backend", "VLC video backend");
    }
    Q_UNREACHABLE();
    return {};
}

} // namespace Settings

#include "moc_VideoPlayerSelectorDialog.cpp"
