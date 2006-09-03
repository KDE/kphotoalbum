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
    void setMode( UsageMode mode );

protected:
    virtual void keyPressEvent( QKeyEvent* ev );
    QListViewItem* findItemInListView( const QString& startWith );
    bool isSpecialKey( QKeyEvent* );
    void handleSpecialKeysInSearch( QKeyEvent* );
    bool itemMatchesText( QListViewItem* item, const QString& text );
    void selectPrevNextMatch( bool next );
    void selectItemAndUpdateLineEdit( QListViewItem* item, int itemStart, const QString& inputText );
    void mergePreviousImageSelection();

private:
    QListView* _listView;
    UsageMode _mode;
    ListSelect* _listSelect;
    bool _showingSelectionOnly;
};

}

#endif /* ANNOTATIONDIALOG_COMPLETABLELINEEDIT_H */

