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

#ifndef IMPORTMATCHER_H
#define IMPORTMATCHER_H

#include <qscrollview.h>
class QGridLayout;
class QComboBox;
class QCheckBox;

namespace ImportExport
{

class CategoryMatch  {
public:
    CategoryMatch( bool allowNew, const QString& categort, QStringList items, QWidget* parent, QGridLayout* grid, int row );
    QCheckBox* _checkbox;
    QComboBox* _combobox;
    QString _text;
};


class ImportMatcher :public QScrollView {
    Q_OBJECT

public:
    ImportMatcher( const QString& otherCategory, const QString& myCategory,
                   const QStringList& otherItems, const QStringList& myItems,
                   bool allowNew, QWidget* parent, const char* name = 0 );

    QString _otherCategory;
    QString _myCategory;
    QValueList<CategoryMatch*> _matchers;
};

}

#endif /* IMPORTMATCHER_H */

