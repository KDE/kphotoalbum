/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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
#include "PluginsPage.h"

#include <KLocalizedString>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <config-kpa-plugins.h>

#ifdef HASKIPI
#include <KIPI/ConfigWidget>
#include <KIPI/PluginLoader>
#endif

#include "SettingsData.h"

#include <MainWindow/Window.h>

Settings::PluginsPage::PluginsPage(QWidget *parent)
    : QWidget(parent)
{
    // TODO: DEPENDENCY: the circular dependency on mainwindow is unfortunate.
    ::MainWindow::Window::theMainWindow()->loadKipiPlugins();
    QVBoxLayout *lay1 = new QVBoxLayout(this);

    QLabel *label = new QLabel(i18n("Choose Plugins to load:"), this);
    lay1->addWidget(label);

    m_pluginConfig = KIPI::PluginLoader::instance()->configWidget(this);
    lay1->addWidget(m_pluginConfig);

    m_delayLoadingPlugins = new QCheckBox(i18n("Delay loading plugins until the plugin menu is opened"), this);
    lay1->addWidget(m_delayLoadingPlugins);
}

void Settings::PluginsPage::saveSettings(Settings::SettingsData *opt)
{
    m_pluginConfig->apply();
    opt->setDelayLoadingPlugins(m_delayLoadingPlugins->isChecked());
}

void Settings::PluginsPage::loadSettings(Settings::SettingsData *opt)
{
    m_delayLoadingPlugins->setChecked(opt->delayLoadingPlugins());
}
// vi:expandtab:tabstop=4 shiftwidth=4:
