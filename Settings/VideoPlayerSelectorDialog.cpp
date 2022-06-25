// SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoPlayerSelectorDialog.h"
#include "Logging.h"
#include <config-kpa-videobackends.h>

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
                       "on many different video formats - possibly because our system wasn't correctly configured. "
                       "If this is the case for you (and you can't use VLC), then try the <b>QtAV</b> back-end, "
                       "which seems to support a lot of formats (everything we threw at it). "
                       "<b>QtAV</b> unfortunately also has a drawback - it seems to get slightly out of sync between video and audio.</p>"

                       "<p><b><font color=red>You can at any time change the backend from Settings -> Viewer</font></b></p></html>");
    label = new QLabel(txt);
    label->setWordWrap(true);
    layout->addWidget(label);

    bool somethingNotAvailable = false;
    if (availableVideoBackends().testFlag(VideoBackend::VLC)) {
        m_vlc = new QRadioButton(QString::fromUtf8("VLC"));
    } else {
        m_vlc = new QRadioButton(QString::fromUtf8("VLC") + i18n(" (NOT AVAILABLE)"));
        m_vlc->setEnabled(false);
        somethingNotAvailable = true;
    }
    layout->addWidget(m_vlc);

    if (availableVideoBackends().testFlag(VideoBackend::QtAV)) {
        m_qtav = new QRadioButton(QString::fromUtf8("QtAV"));
    } else {
        m_qtav = new QRadioButton(QString::fromUtf8("QtAV") + i18n(" (NOT AVAILABLE)"));
        m_qtav->setEnabled(false);
        somethingNotAvailable = true;
    }
    layout->addWidget(m_qtav);

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
        case VideoBackend::Phonon:
            candidate = m_phonon;
            break;
        case VideoBackend::QtAV:
            candidate = m_qtav;
            break;
        case VideoBackend::VLC:
            candidate = m_vlc;
            break;
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
    if (m_vlc->isChecked())
        return VideoBackend::VLC;
    else if (m_qtav->isChecked())
        return VideoBackend::QtAV;
    else
        return VideoBackend::Phonon;
}

constexpr VideoBackends availableVideoBackends()
{
    VideoBackends availableBackends;
#if LIBVLC_FOUND
    availableBackends |= VideoBackend::VLC;
#endif
#if QtAV_FOUND
    availableBackends |= VideoBackend::QtAV;
#endif
#if Phonon4Qt5_FOUND
    availableBackends |= VideoBackend::Phonon;
#endif
    static_assert(LIBVLC_FOUND || QtAV_FOUND || Phonon4Qt5_FOUND, "A video backend must be provided. The build system should bail out if none is available.");
    return availableBackends;
}

VideoBackend preferredVideoBackend(const VideoBackend configuredBackend, const VideoBackends exclusions)
{
    if (availableVideoBackends().testFlag(configuredBackend) && !exclusions.testFlag(configuredBackend)) {
        qCDebug(SettingsLog) << "preferredVideoBackend(): configured backend is viable:" << configuredBackend;
        return configuredBackend;
    }

    // change backend priority here:
    for (const VideoBackend candidate : { VideoBackend::Phonon, VideoBackend::VLC, VideoBackend::QtAV }) {
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
    case VideoBackend::NotConfigured:
        return i18nc("A friendly name for the video backend", "Unconfigured video backend");
    case VideoBackend::Phonon:
        return i18nc("A friendly name for the video backend", "Phonon video backend");
    case VideoBackend::QtAV:
        return i18nc("A friendly name for the video backend", "QtAV video backend");
    case VideoBackend::VLC:
        return i18nc("A friendly name for the video backend", "VLC video backend");
    }
    Q_UNREACHABLE();
    return {};
}

} // namespace Settings

#include "moc_VideoPlayerSelectorDialog.cpp"
