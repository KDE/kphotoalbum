// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ViewerPage.h"

#include "VideoPlayerSelectorDialog.h"
#include "ViewerSizeConfig.h"

#include <kpabase/SettingsData.h>

#include <KComboBox>
#include <KLocalizedString>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

static QString videoBackendTextFromEnum(Settings::VideoBackend backend)
{
    switch (backend) {
    case Settings::VideoBackend::NotConfigured:
        return i18n("Not Configured");
    case Settings::VideoBackend::VLC:
        return QString::fromUtf8("VLC");
    case Settings::VideoBackend::QtAV: // legacy value; no longer used actively
        return QString::fromUtf8("QtAV");
    case Settings::VideoBackend::Phonon:
        return QString::fromUtf8("Phonon");
    }
    Q_UNREACHABLE();
    return {}; // Make CI shut up.
}

Settings::ViewerPage::ViewerPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay1 = new QVBoxLayout(this);

    m_slideShowSetup = new ViewerSizeConfig(i18n("Running Slide Show From Thumbnail View"), this);
    lay1->addWidget(m_slideShowSetup);

    m_viewImageSetup = new ViewerSizeConfig(i18n("Viewing Images and Videos From Thumbnail View"), this);
    lay1->addWidget(m_viewImageSetup);

    QGridLayout *glay = new QGridLayout;
    lay1->addLayout(glay);
    int row = -1;

    QLabel *label = new QLabel(i18n("Slideshow interval:"), this);
    glay->addWidget(label, ++row, 0);

    m_slideShowInterval = new QSpinBox;
    m_slideShowInterval->setRange(1, INT_MAX);
    glay->addWidget(m_slideShowInterval, row, 1);
    m_slideShowInterval->setSuffix(i18n(" sec"));
    label->setBuddy(m_slideShowInterval);

    label = new QLabel(i18n("Image cache:"), this);
    glay->addWidget(label, ++row, 0);

    m_cacheSize = new QSpinBox;
    m_cacheSize->setRange(0, 16384);
    m_cacheSize->setSingleStep(10);
    m_cacheSize->setSuffix(i18n(" Mbytes"));
    glay->addWidget(m_cacheSize, row, 1);
    label->setBuddy(m_cacheSize);

    QString txt;

    QLabel *standardSizeLabel = new QLabel(i18n("Standard size in viewer:"), this);
    m_viewerStandardSize = new KComboBox(this);
    m_viewerStandardSize->addItems(QStringList() << i18n("Full Viewer Size") << i18n("Natural Image Size") << i18n("Natural Image Size If Possible"));
    glay->addWidget(standardSizeLabel, ++row, 0);
    glay->addWidget(m_viewerStandardSize, row, 1);
    standardSizeLabel->setBuddy(m_viewerStandardSize);

    txt = i18n("<p>Set the standard size for images to be displayed in the viewer.</p> "
               "<p><b>Full Viewer Size</b> indicates that the image will be stretched or shrunk to fill the viewer window.</p> "
               "<p><b>Natural Image Size</b> indicates that the image will be displayed pixel for pixel.</p> "
               "<p><b>Natural Image Size If Possible</b> indicates that the image will be displayed pixel for pixel if it would fit the window, "
               "otherwise it will be shrunk to fit the viewer.</p>");
    m_viewerStandardSize->setWhatsThis(txt);

    QLabel *scalingLabel = new QLabel(i18n("Scaling algorithm:"), this);
    m_smoothScale = new KComboBox(this);
    m_smoothScale->addItems(QStringList() << i18n("Fastest") << i18n("Best"));
    scalingLabel->setBuddy(m_smoothScale);

    glay->addWidget(scalingLabel, ++row, 0);
    glay->addWidget(m_smoothScale, row, 1);
    txt = i18n("<p>When displaying images, KPhotoAlbum normally performs smooth scaling of the image. "
               "If this option is not set, KPhotoAlbum will use a faster but less smooth scaling method.</p>");
    scalingLabel->setWhatsThis(txt);
    m_smoothScale->setWhatsThis(txt);

    label = new QLabel(i18n("Video Backend:"), this);
    m_videoBackendButton = new QPushButton;
    glay->addWidget(label, ++row, 0);
    glay->addWidget(m_videoBackendButton, row, 1);
    connect(m_videoBackendButton, &QPushButton::clicked, this, [this] {
        VideoPlayerSelectorDialog dialog(this);
        dialog.exec();
        m_videoBackend = dialog.backend();
        m_videoBackendButton->setText(videoBackendTextFromEnum(m_videoBackend));
    });
}

void Settings::ViewerPage::loadSettings(Settings::SettingsData *opt)
{
    m_viewImageSetup->setLaunchFullScreen(opt->launchViewerFullScreen());
    m_viewImageSetup->setSize(opt->viewerSize());
    m_slideShowSetup->setLaunchFullScreen(opt->launchSlideShowFullScreen());
    m_slideShowSetup->setSize(opt->slideShowSize());
    m_slideShowInterval->setValue(opt->slideShowInterval());
    m_cacheSize->setValue(opt->viewerCacheSize());
    m_smoothScale->setCurrentIndex(opt->smoothScale());
    m_viewerStandardSize->setCurrentIndex(opt->viewerStandardSize());
    m_videoBackend = opt->videoBackend();
    m_videoBackendButton->setText(videoBackendTextFromEnum(m_videoBackend));
}

void Settings::ViewerPage::saveSettings(Settings::SettingsData *opt)
{
    opt->setLaunchViewerFullScreen(m_viewImageSetup->launchFullScreen());
    opt->setViewerSize(m_viewImageSetup->size());
    opt->setSlideShowInterval(m_slideShowInterval->value());
    opt->setViewerCacheSize(m_cacheSize->value());
    opt->setSmoothScale(m_smoothScale->currentIndex());
    opt->setViewerStandardSize((StandardViewSize)m_viewerStandardSize->currentIndex());
    opt->setSlideShowSize(m_slideShowSetup->size());
    opt->setLaunchSlideShowFullScreen(m_slideShowSetup->launchFullScreen());
    opt->setVideoBackend(m_videoBackend);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
