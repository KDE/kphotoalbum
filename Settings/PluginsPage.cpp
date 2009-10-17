#include "PluginsPage.h"
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <MainWindow/Window.h>
#include <config-kpa-kipi.h>
#ifdef HASKIPI
#  include <libkipi/pluginloader.h>
#endif

Settings::PluginsPage::PluginsPage( QWidget* parent )
    : QWidget(parent)
{
    // TODO: DEPENDENCY: the circular dependency on mainwindow is unfortunate.
    ::MainWindow::Window::theMainWindow()->loadPlugins();
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    QLabel* label = new QLabel( i18n("Choose Plugins to load:"), this );
    lay1->addWidget( label );

    _pluginConfig = KIPI::PluginLoader::instance()->configWidget( this );
    lay1->addWidget( _pluginConfig );

    _delayLoadingPlugins = new QCheckBox( i18n("Delay loading plugins until the plugin menu is opened"), this );
    lay1->addWidget( _delayLoadingPlugins );
}

void Settings::PluginsPage::saveSettings( Settings::SettingsData* opt )
{
    _pluginConfig->apply();
    opt->setDelayLoadingPlugins( _delayLoadingPlugins->isChecked() );
}

void Settings::PluginsPage::loadSettings( Settings::SettingsData* opt )
{
    _delayLoadingPlugins->setChecked( opt->delayLoadingPlugins() );
}
