/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef BROWSER_H
#define BROWSER_H
#include <qlistview.h>
#include <qiconview.h>
#include "options.h"
class FolderAction;
class ImageSearchInfo;
class QListViewItem;
class BrowserItemFactory;
class QWidgetStack;

class Browser :public QWidget {
    Q_OBJECT
    friend class ImageFolderAction;

public:
    Browser( QWidget* parent, const char* name = 0 );
    ~Browser();
    void addSearch( ImageSearchInfo& info );
    static Browser* instance();
    void load( const QString& optionGroup, const QString& value );
    bool allowSort();
    ImageSearchInfo currentContext();
    void clear();
    void setFocus();
    QString currentCategory() const;

public slots:
    void back();
    void forward();
    void go();
    void home();
    void reload();
    void slotSmallListView();
    void slotLargeListView();
    void slotSmallIconView();
    void slotLargeIconView();

signals:
    void canGoBack( bool );
    void canGoForward( bool );
    void showingOverview();
    void pathChanged( const QString& );
    void showsContentView( bool );
    void currentSizeAndTypeChanged( Options::ViewSize, Options::ViewType );

protected slots:
    void init();
    void select( QListViewItem* item );
    void select( QIconViewItem* item );
    void select( FolderAction* action );

protected:
    void addItem( FolderAction* );
    void emitSignals();
    void setupFactory();
    void setSizeAndType( Options::ViewType type, Options::ViewSize size );

private:
    static Browser* _instance;
    QValueList<FolderAction*> _list;
    int _current;
    BrowserItemFactory* _listViewFactory;
    BrowserItemFactory* _iconViewFactory;
    BrowserItemFactory* _currentFactory;
    QWidgetStack* _stack;
    QIconView* _iconView;
    QListView* _listView;
};


#endif /* BROWSER_H */

