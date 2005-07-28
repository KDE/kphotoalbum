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

#include "donate.h"
#include <qlayout.h>
#include <qlabel.h>
#include <krun.h>
#include <kurl.h>
#include <klocale.h>
#include <kstandarddirs.h>

Donate::Donate( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Donate Money"), Close | User1, Close, parent, name )
{
    QWidget* top = plainPage();
    QHBoxLayout* layout = new QHBoxLayout( top, 10 );

    QLabel* image = new QLabel( top, "image" );
    image->setMinimumSize( QSize( 273, 204 ) );
    image->setMaximumSize( QSize( 273, 204 ) );
    image->setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/splash.png") ) );
    layout->addWidget( image );

    QString txt = i18n("<qt><p><center><b>Donate Money to the KimDaBa developer</b></center></p>"

                       "<p>My digital camera is starting to get old "
                       "so I hope that all of you would show your appreciation of my work, by helping me buy a new one, that will "
                       "ensure that I enjoy taking pictures - and sorting them for many years to come.</p>"

                       "<p>Money may be donated by clicking on the Donate button below, which will take you "
                       "to PayPal.</p>"

                       "<p>Alternatively you may transfer money to:<br/>Jesper Pedersen<br/>Danske Bank, Denmark.<br/>"
                       "SWIFT code is DABADKKK<br/>reg no: 3203<br/>account no: 3593401928<br>"
                       "(Please email blackie@kde.org a notification)</p>"
                       "<p>Any donation is highly appreciated.</p></qt>");
    QLabel* label = new QLabel( txt, top );
    layout->addWidget( label );
    setButtonText( User1, i18n("Donate") );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotDonate() ) );
}

void Donate::slotDonate()
{
    KRun::runURL(KURL(QString::fromLatin1("https://www.paypal.com/xclick/business=blackie%40blackie.dk&item_name=KimDaBa")),
                 QString::fromLatin1( "text/html" ) );
}

#include "donate.moc"
