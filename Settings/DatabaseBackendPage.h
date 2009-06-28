#ifndef DATABASEBACKENDPAGE_H
#define DATABASEBACKENDPAGE_H
#include <QWidget>
class Q3ButtonGroup;
class QCheckBox;
class QSpinBox;

namespace SQLDB { class SQLSettingsWidget; }


namespace Settings
{
class SettingsData;

class DatabaseBackendPage :public QWidget
{
public:
    DatabaseBackendPage( QWidget* parent );
    void loadSettings( Settings::SettingsData* );
    void saveSettings( Settings::SettingsData* );

private:
    QSpinBox* _autosave;
    QSpinBox* _backupCount;
    QCheckBox* _compressBackup;
    QCheckBox* _compressedIndexXML;
    Q3ButtonGroup* _backendButtons;
    SQLDB::SQLSettingsWidget* _sqlSettings;
};

}

#endif /* DATABASEBACKENDPAGE_H */

