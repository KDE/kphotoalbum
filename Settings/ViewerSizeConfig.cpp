/* SPDX-FileCopyrightText: 2003-2018 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ViewerSizeConfig.h"

#include <KLocalizedString>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

Settings::ViewerSizeConfig::ViewerSizeConfig(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    setLayout(topLayout);
    m_fullScreen = new QCheckBox(i18n("Launch in full screen"), this);
    topLayout->addWidget(m_fullScreen);

    QWidget *sizeBox = new QWidget(this);
    topLayout->addWidget(sizeBox);
    QHBoxLayout *lay = new QHBoxLayout(sizeBox);

    QLabel *label = new QLabel(i18n("Size:"), sizeBox);
    lay->addWidget(label);

    m_width = new QSpinBox;
    m_width->setRange(100, 5000);
    m_width->setSingleStep(50);
    lay->addWidget(m_width);

    label = new QLabel(QString::fromLatin1("x"), sizeBox);
    lay->addWidget(label);

    m_height = new QSpinBox;
    m_height->setRange(100, 5000);
    m_height->setSingleStep(50);
    lay->addWidget(m_height);

    lay->addStretch(1);
    topLayout->addStretch(1);
}

void Settings::ViewerSizeConfig::setSize(const QSize &size)
{
    m_width->setValue(size.width());
    m_height->setValue(size.height());
}

QSize Settings::ViewerSizeConfig::size()
{
    return QSize(m_width->value(), m_height->value());
}

void Settings::ViewerSizeConfig::setLaunchFullScreen(bool b)
{
    m_fullScreen->setChecked(b);
}

bool Settings::ViewerSizeConfig::launchFullScreen() const
{
    return m_fullScreen->isChecked();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
