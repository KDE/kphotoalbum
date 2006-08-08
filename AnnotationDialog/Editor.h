/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef EDITOR_H
#define EDITOR_H
#include <qtextedit.h>

class KDictSpellingHighlighter;
class KSpellConfig;
class QPoint;
class QPopupMenu;

namespace AnnotationDialog
{

class Editor :public QTextEdit
{
    Q_OBJECT

public:
    Editor( QWidget* parent, const char* name = 0 );

protected:
    virtual QPopupMenu* createPopupMenu( const QPoint & pos );
    QString wordAtPos( const QPoint& pos );
    QPopupMenu* replacementMenu( const QString& word );
    virtual void contentsContextMenuEvent( QContextMenuEvent *e );
    void replaceWord( const QPoint& pos, const QString& replacement );
    bool wordBoundaryAtPos( const QPoint& pos, int* para, int* start, int* end );
    void fetchDicts( QStringList* titles, QStringList* dicts );
    void createHighlighter();
    virtual void keyPressEvent( QKeyEvent* );

protected slots:
    void addSuggestion(const QString&, const QStringList&, unsigned int);
    void itemSelected( int );

private:
    QMap<QString,QStringList> _replacements;
    QString _currentWord;
    KSpellConfig* _config;
    KDictSpellingHighlighter* _highlighter;
};

}

#endif /* EDITOR_H */

