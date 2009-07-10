/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef BREADCRUMB_H
#define BREADCRUMB_H
#include <QString>

namespace Browser
{

/**
 * \brief Information about a single breadcrumb
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This is basically a simple class to make the code for handling
 * breadcrumbs simpler. It encodes the following two informations about a
 * breadcrumb:
 * \li Is this a first breadcrumb (the result of going home e.g.)
 * \li which text should be shown for this breadcrumb.
 *
 */
class Breadcrumb
{
public:
    static Breadcrumb empty();
    static Breadcrumb home();
    static Breadcrumb view();

    Breadcrumb( const QString& text, bool isBeginning = false );
    QString text() const;
    bool isBeginning() const;
    bool isView() const;
    bool operator==( const Breadcrumb& other ) const;
    bool operator!=( const Breadcrumb& other ) const;

private:
    int _index;
    bool _isBeginning;
    bool _isView;
    QString _text;
    static int _count;
};

}

#endif /* BREADCRUMB_H */

