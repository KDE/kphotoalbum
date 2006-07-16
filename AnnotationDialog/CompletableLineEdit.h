#ifndef ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H
#define ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H

#include <qlineedit.h>
#include "ListSelect.h"

namespace AnnotationDialog
{
class CompletableLineEdit :public QLineEdit {
public:
    CompletableLineEdit( ListSelect* parent,  const char* name = 0 );
    void setListView( QListView* );
    void setMode( ListSelect::Mode mode );

protected:
    virtual void keyPressEvent( QKeyEvent* ev );
    QListViewItem* findItemInListView( const QString& startWith );
    bool isSpecialKey( QKeyEvent* );
    void handleSpecialKeysInSearch( QKeyEvent* );
    void showOnlyItemsMatching( const QString& text );
    bool itemMatchesText( QListViewItem* item, const QString& text );
    void selectPrevNextMatch( bool next );
    void selectItemAndUpdateLineEdit( QListViewItem* item, int itemStart, const QString& inputText );

private:
    QListView* _listView;
    ListSelect::Mode _mode;
    ListSelect* _listSelect;
};

}

#endif /* ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H */

