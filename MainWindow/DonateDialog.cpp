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

#include "DonateDialog.h"
#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <krun.h>
#include <kurl.h>
#include <klocale.h>
#include <kstandarddirs.h>

using namespace MainWindow;

DonateDialog::DonateDialog( QWidget* parent, const char* name )
#ifdef TEMPORARILY_REMOVED
    :KDialog( Plain, i18n("Donate Money"), Close | User1, Close, parent, name )
#endif
{
#ifdef TEMPORARILY_REMOVED
#ifdef TEMPORARILY_REMOVED
    QWidget* top = plainPage();
    Q3HBoxLayout* layout = new Q3HBoxLayout( top, 10 );

    QLabel* image = new QLabel( top, "image" );
    image->setMinimumSize( QSize( 273, 204 ) );
    image->setMaximumSize( QSize( 273, 204 ) );
    image->setPixmap( KStandardDirs::locate("data", QString::fromLatin1("kphotoalbum/pics/splash.png") ) );
    layout->addWidget( image );

    QString txt = i18n("<p><center><b>Donate Money</b></center></p>"

                       "<p>KPhotoAlbum is in the order of magnitude 2 man years of work in my spare time. "
                       "This work is completely unpaid, and I do it just for the fun of it. "
                       "Having said that, I would be more than happy for any donation you might want to offer small or big.</p>"
                       "<p>The money is likely used for one of these things:"
                       "<ul><li>Buying something cool for my camera - which makes me want to "
                       "adapt KPhotoAlbum to using that new feature (EXIF support and video support are two good examples of this)."
                       "<li>KPhotoAlbum PR (the splashscreen contest is a good example of that).</ul></p>");
    QLabel* label = new QLabel( txt, top );
    layout->addWidget( label );
    setButtonText( User1, i18n("Donate") );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotDonate() ) );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

void DonateDialog::slotDonate()
{
#ifdef TEMPORARILY_REMOVED
    KRun::runURL(KUrl(QString::fromLatin1("https://www.paypal.com/xclick/business=blackie%40blackie.dk&item_name=KimDaBa")),
                 QString::fromLatin1( "text/html" ) );
#else
    kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

#include "DonateDialog.moc"
