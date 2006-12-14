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
#include "MySurvey.h"
#include "TextQuestion.h"
#include "SurveyCount.h"
#include "InfoPage.h"
#include <klocale.h>
#include "AlternativeQuestion.h"

using namespace Survey;

MySurvey::MySurvey( QWidget* parent, const char* name )
    :Survey::SurveyDialog( parent, name )
{
    setSurveyVersion( 2, 1 );
    setReceiver( QString::fromLatin1( "blackie@kde.org" ) );

    QStringList yesNoList;
    yesNoList << i18n("Yes") << i18n("No");

    QString txt
        = i18n("<p>This is the KPhotoAlbum survey. Its goal is to collect the "
               "information needed to make KPhotoAlbum meet the needs of users like you.</p>"
               "<p>Spend a couple of minutes and share your experience of using KPhotoAlbum.</p>"
               "<p>At any time, you can quit the survey and return to "
               "it later on; it will remember your previous answers.</p>"
               "<p align=\"right\">Thanks in advance! Jesper</p>");

    Survey::InfoPage* front = new Survey::InfoPage( QString::fromLatin1("KPhotoAlbum"), txt, Survey::InfoPage::Front, this );
    setFrontPage( front );

    new Survey::RadioButtonQuestion( QString::fromLatin1( "HowLongHaveYouUsedKPhotoAlbum" ),
                                     i18n("Length of Usage"),
                                     QString::null,
                                     i18n("How long have you used KPhotoAlbum?"),
                                     QStringList() << i18n("< 1 Month") << i18n("1-6 Months") << i18n("6-12 Months")
                                     << i18n("1-2 Year" ) << i18n("> 2 Years"), this );

    new SurveyCountQuestion( QString::fromLatin1( "ImageCount" ),
                             i18n("Image Count"),
                             this );

    new Survey::AlternativeQuestion( QString::fromLatin1( "WhoAreYou" ), i18n("User demographics"),
                                     i18n("In designing the software, it's important to know who uses it. "
                                          "You can choose any number of categories from the list below." ),
                                     i18n("Which of these categories (if any) do you fit in?"),
                                     QStringList()
                                     << i18n( "I'm a professional software developer" )
                                     << i18n( "I'm a professional photographer" )
                                     << i18n( "I'm a C++ programmer" )
                                     << i18n( "I'm a student studying software development" )
                                     << i18n( "I'm a student studying something else" ), 0, Survey::AlternativeQuestion::CheckBox, this );


    QStringList kphotoalbumUsage;
    kphotoalbumUsage << i18n("Private albums") << i18n("Professional");

    new Survey::AlternativeQuestion( QString::fromLatin1( "WhatAreYouUsingKPhotoAlbumFor" ),
                                     i18n( "For what are you using KPhotoAlbum?" ),
                                     i18n( "If you are using KPhotoAlbum professionally, please send an email to me at blackie@kde.org "
                                           "explaining how you are using KPhotoAlbum, and how I may improve it "
                                           "in ways that would make it more valuable for professionals."),
                                     i18n( "For what are you using KPhotoAlbum?"),
                                     kphotoalbumUsage, 2, Survey::AlternativeQuestion::CheckBox, this );

    new Survey::TextQuestion( QString::fromLatin1( "Comment" ),
                              i18n("General Comments"),
                              i18n("<p>Every time my email client indicate a survey has been completed I can't wait to the end of my "
                                   "working day so I can do some KPhotoAlbum programming.</p>"
                                   "<p>One problem with software development is that developers most often hears about problems, and "
                                   "seldom about how much people enjoy the applications. "
                                   "This is your chance to give me some positive energy by telling me how much you like the application.</p>"),
                              this );

    txt = i18n("<p>Thank you very much for your time, I hope you will continue using KPhotoAlbum for many years to come, "
               "and that future versions will fit your purpose even better than it does today.</p>"
               "Finally, allow me to ask you to consider giving a donation. "
               "See the Help->Donation menu on how to make a donation.</p>"
               "<p align=\"right\">Once again thanks for filling out this survey - Jesper</p>");

    Survey::InfoPage* back = new Survey::InfoPage( QString::fromLatin1("KPhotoAlbum"), txt, Survey::InfoPage::Back, this );
    setBackPage( back );


}

QSize MySurvey::sizeHint() const
{
    QSize size = Survey::SurveyDialog::sizeHint();
    return QSize( QMAX( size.width(), 800 ), size.height() );
}

#include "MySurvey.moc"
