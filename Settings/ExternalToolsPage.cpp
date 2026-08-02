// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ExternalToolsPage.h"

#include <kpabase/SettingsData.h>

#include <KLocalizedString>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

Settings::ExternalToolsPage::ExternalToolsPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *imageLayout = new QHBoxLayout(this);
    imageLayout->addWidget(new QLabel(i18n("Image Tool:"), this));
    m_imageToolRequester = new KUrlRequester();
    m_imageToolRequester->setPlaceholderText(i18n("Not configured"));
    imageLayout->addWidget(m_imageToolRequester);
    topLayout->addLayout(imageLayout);

    QHBoxLayout *videoLayout = new QHBoxLayout(this);
    videoLayout->addWidget(new QLabel(i18n("Video Tool:"), this));
    m_videoToolRequester = new KUrlRequester();
    m_videoToolRequester->setPlaceholderText(i18n("Not configured"));
    videoLayout->addWidget(m_videoToolRequester);
    topLayout->addLayout(videoLayout);

    topLayout->addStretch(1);
}

void Settings::ExternalToolsPage::loadSettings(Settings::SettingsData *opt)
{
    m_imageToolRequester->setText(opt->externalImageTool());
    m_videoToolRequester->setText(opt->externalVideoTool());
}

void Settings::ExternalToolsPage::saveSettings(Settings::SettingsData *opt)
{
    opt->setExternalImageTool(m_imageToolRequester->text());
    opt->setExternalVideoTool(m_videoToolRequester->text());
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ExternalToolsPage.cpp"
