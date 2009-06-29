#ifndef UNTAGGEDGROUPBOX_H
#define UNTAGGEDGROUPBOX_H
#include <QGroupBox>
class QComboBox;

namespace Settings
{
class SettingsData;

class UntaggedGroupBox :public QGroupBox
{
    Q_OBJECT
public:
    UntaggedGroupBox( QWidget* parent );
    void loadSettings( Settings::SettingsData* opt );
    void saveSettings( Settings::SettingsData* opt );

private slots:
    void populateCategoryComboBox();
    void populateTagsCombo();

private:
    QComboBox* _category;
    QComboBox* _tag;
};

}

#endif /* UNTAGGEDGROUPBOX_H */

