#ifndef SUBCATEGORIESPAGE_H
#define SUBCATEGORIESPAGE_H
#include <QWidget>
#include <DB/MemberMap.h>
class Q3ListBoxItem;
class QPushButton;
class Q3ListBox;
class QComboBox;

namespace Settings
{

class SubCategoriesPage :public QWidget
{
    Q_OBJECT

public:
    SubCategoriesPage( QWidget* parent );
    void saveSettings();
    void loadSettings();
    DB::MemberMap* memberMap();

public slots:
    void slotPageChange();
    void categoryRenamed( const QString& oldName, const QString& newName );


private slots:
    void slotCategoryChanged( const QString& );
    void slotGroupSelected( Q3ListBoxItem* );
    void slotAddGroup();
    void slotDelGroup();
    void slotRenameGroup();

private:
    void slotCategoryChanged( const QString&, bool saveGroups );
    void saveOldGroup();
    void selectMembers( const QString& );
    void setButtonStates();

private:
    QComboBox* _category;
    Q3ListBox* _groups;
    Q3ListBox* _members;
    QPushButton* _rename;
    QPushButton* _del;
    DB::MemberMap _memberMap;
    QString _currentCategory;
    QString _currentGroup;
};

}


#endif /* SUBCATEGORIESPAGE_H */

