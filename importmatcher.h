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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef IMPORTMATCHER_H
#define IMPORTMATCHER_H

#include <qscrollview.h>
class QGridLayout;
class QComboBox;
class QCheckBox;

class OptionMatch  {
public:
    OptionMatch( bool allowNew, const QString& optioin, QStringList myOptionList, QWidget* parent, QGridLayout* grid, int row );
    QCheckBox* _checkbox;
    QComboBox* _combobox;
    QString _text;
};


class ImportMatcher :public QScrollView {
    Q_OBJECT

public:
    ImportMatcher( const QString& otherOptionGroup, const QString& myOptionGroup,
                   const QStringList& otherOptionList, const QStringList& myOptionList,
                   bool allowNew, QWidget* parent, const char* name = 0 );

    QString _otherOptionGroup;
    QString _myOptionGroup;
    QValueList<OptionMatch*> _matchers;
};


#endif /* IMPORTMATCHER_H */

