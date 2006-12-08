/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
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

