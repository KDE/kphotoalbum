/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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
class FolderAction;
class ImageSearchInfo;
class QListViewItem;

class Browser :public QListView {
    Q_OBJECT
    friend class ImageFolderAction;

public:
    Browser( QWidget* parent, const char* name = 0 );
    void addSearch( ImageSearchInfo& info );
    static Browser* theBrowser();
    void load( const QString& optionGroup, const QString& value );
    bool allowSort();

public slots:
    void back();
    void forward();
    void go();
    void home();
    void reload();

signals:
    void canGoBack( bool );
    void canGoForward( bool );
    void showingOverview();
    void pathChanged( const QString& );


protected slots:
    void init();
    void select( QListViewItem* item );

protected:
    void addItem( FolderAction* );
    void emitSignals();

private:
    static Browser* _instance;
    QValueList<FolderAction*> _list;
    int _current;
};


#endif /* BROWSER_H */

