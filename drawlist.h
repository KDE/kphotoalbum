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

#ifndef DRAWLIST_H
#define DRAWLIST_H
class Draw;
class QWidget;
#include <qvaluelist.h>
#include <qdom.h>

class DrawList :public QValueList<Draw*>
{
public:
    DrawList();
    DrawList( const DrawList& other );
    ~DrawList();
    DrawList& operator=( const DrawList& other );
    void load( QDomElement elm );
    void save( QDomDocument doc, QDomElement top );

protected:
    void deleteItems();
};

#endif /* DRAWLIST_H */

