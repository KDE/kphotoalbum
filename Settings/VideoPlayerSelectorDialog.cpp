// SPDX-FileCopyrightText: 2021 The KPhotoAlbum Development Team
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoPlayerSelectorDialog.h"
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

#include <config-kphotoalbum.h>

namespace Settings
{

VideoPlayerSelectorDialog::VideoPlayerSelectorDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(i18n("<h1>Choose a Video Player</h1>"));
    layout->addWidget(label);

    QString txt = i18n("<html><p>There are unfortunately no video player which just works out of the box for everyone.</p>"
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
#if LIBVLC_FOUND
    m_vlc = new QRadioButton(QString::fromUtf8("VLC"));
#else
    m_vlc = new QRadioButton(QString::fromUtf8("VLC") + i18n(" (NOT AVILABLE)"));
    m_vlc->setEnabled(false);
    somethingNotAvailable = true;
#endif
    layout->addWidget(m_vlc);

#if QtAV_FOUND
    m_qtav = new QRadioButton(QString::fromUtf8("QtAV"));
#else
    m_qtav = new QRadioButton(QString::fromUtf8("QtAV") + i18n(" (NOT AVILABLE)"));
    m_qtav->setEnabled(false);
    somethingNotAvailable = true;
#endif
    layout->addWidget(m_qtav);

#if Phonon4Qt5_FOUND
    m_phonon = new QRadioButton(QString::fromUtf8("Phonon"));
#else
    m_phonon = new QRadioButton(QString::fromUtf8("Phonon") + i18n(" (NOT AVILABLE)"));
    m_phonon->setEnabled(false);
    somethingNotAvailable = true;
#endif
    layout->addWidget(m_phonon);

    auto backend = SettingsData::instance()->videoBackend();
    QRadioButton *rb = [&] {
        QRadioButton *candidate = nullptr;

#if Phonon4Qt5_FOUND
        if (backend == VideoBackend::Phonon)
            return m_phonon;
        candidate = m_phonon;
#endif
#if QtAV_FOUND
        if (backend == VideoBackend::QtAV)
            return m_qtav;
        candidate = m_qtav;
#endif

#if LIBVLC_FOUND
        if (backend == VideoBackend::VLC)
            return m_vlc;
        candidate = m_vlc;
#endif
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

} // namespace Settings
