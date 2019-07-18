/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
