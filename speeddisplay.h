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

#ifndef SPEEDDISPLAY_H
#define SPEEDDISPLAY_H
#include <qdialog.h>
class QTimer;
class QLabel;
class QHBoxLayout;

class SpeedDisplay :public QDialog {
    Q_OBJECT

public:
    SpeedDisplay( QWidget* parent, const char* name = 0 );
    void display( int );
    void start();
    void end();
    void go();

private:
    QTimer* _timer;
    QLabel* _label;
    QHBoxLayout* _layout;
};


#endif /* SPEEDDISPLAY_H */

