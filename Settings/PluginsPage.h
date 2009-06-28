#ifndef PLUGINSPAGE_H
#define PLUGINSPAGE_H
#include <QWidget>

namespace KIPI { class ConfigWidget; }

class QCheckBox;
namespace Settings
{
class SettingsData;

class PluginsPage :public QWidget
{
public:
    PluginsPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );

private:
    KIPI::ConfigWidget* _pluginConfig;
    QCheckBox* _delayLoadingPlugins;
};

}


#endif /* PLUGINSPAGE_H */

