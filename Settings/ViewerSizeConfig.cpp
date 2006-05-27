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

#include "ViewerSizeConfig.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <klocale.h>
#include <qlabel.h>

Settings::ViewerSizeConfig::ViewerSizeConfig( const QString& title, QWidget* parent, const char* name )
    :QVGroupBox( title, parent, name )
{
    _fullScreen = new QCheckBox( i18n("Launch in full screen" ), this );

    QWidget* sizeBox = new QWidget( this );
    QHBoxLayout* lay = new QHBoxLayout( sizeBox, 0, 6 );

    QLabel* label = new QLabel( i18n("Size:"), sizeBox );
    lay->addWidget( label );

    _width = new QSpinBox( 100, 5000, 50, sizeBox );
    lay->addWidget( _width );

    label = new QLabel( QString::fromLatin1("x"), sizeBox );
    lay->addWidget( label );

    _height = new QSpinBox( 100, 5000, 50, sizeBox );
    lay->addWidget( _height );

    lay->addStretch( 1 );
}

void Settings::ViewerSizeConfig::setSize( const QSize& size  )
{
    _width->setValue( size.width() );
    _height->setValue( size.height() );
}

QSize Settings::ViewerSizeConfig::size()
{
    return QSize( _width->value(), _height->value() );
}

void Settings::ViewerSizeConfig::setLaunchFullScreen( bool b )
{
    _fullScreen->setChecked( b );
}

bool Settings::ViewerSizeConfig::launchFullScreen() const
{
    return _fullScreen->isChecked();
}

#include "ViewerSizeConfig.moc"
