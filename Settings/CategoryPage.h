#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H
#include "SettingsData.h"
#include <QLabel>
#include <QWidget>
class Q3ListBoxItem;
class Q3ListBox;
class KPushButton;
class QComboBox;
class QSpinBox;
class KIconButton;
class QLineEdit;

namespace DB { class MemberMap; }

namespace Settings
{
class CategoryItem;
class SettingsDialog;

class CategoryPage :public QWidget
{
    Q_OBJECT
public:
    CategoryPage( QWidget* parent );
    void enableDisable( bool );
    void saveSettings( DB::MemberMap* memberMap );
    void loadSettings();

signals:
    void currentCategoryNameChanged( const QString& oldName, const QString& newName );

private slots:
    void edit( Q3ListBoxItem* );
    void slotLabelChanged( const QString& );
    void slotIconChanged( const QString& );
    void thumbnailSizeChanged( int );
    void slotPreferredViewChanged( int );
    void slotNewItem();
    void slotDeleteCurrent();

private:
    Q3ListBox* _categories;
    QLabel* _labelLabel;
    QLineEdit* _text;
    QLabel* _iconLabel;
    KIconButton* _icon;
    QLabel* _thumbnailSizeInCategoryLabel;
    QSpinBox* _thumbnailSizeInCategory;
    QLabel* _preferredViewLabel;
    QComboBox* _preferredView;
    KPushButton* _delItem;
    Settings::CategoryItem* _current;
    QList<CategoryItem*> _deleted;
};

}

#endif /* CATEGORYPAGE_H */

