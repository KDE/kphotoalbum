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
        = i18n("<qt><p>This is the KPhotoAlbum survey, its intention is to make KPhotoAlbum fit the need <i>you</i> may have.</p>"
               "I have spent most of my sparetime on KPhotoAlbum the last three and a half year, so I'd be really happy if you "
               "could spent the next five to ten minute telling me what you think about it, and giving me some "
               "feedback on which part of the application you are using.</p>"
               "<p>At any time you may quit the survey and get back to it later, it will remember your answers</p>"
               "<p align=\"right\">Thanks in advance - Jesper</p></qt>");

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

    new Survey::AlternativeQuestion( QString::fromLatin1( "WhoAreYou" ), i18n("Who Are You"),
                                     i18n("I'd like you know what kind of people are using KPhotoAlbum, please let me know which "
                                          "of these categories (if any) you feel you fit in" ),
                                     i18n("Which of these categories (if any) do you fit in?"),
                                     QStringList()
                                     << i18n( "I'm a professional software developer" )
                                     << i18n( "I'm a professional photographer" )
                                     << i18n( "I'm a C++ programer" )
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

    new Survey::RadioButtonQuestion( QString::fromLatin1( "WhoAreYouTake2" ),
                                     i18n( "Your Approach to Digital Images" ),
                                     i18n( "I don't really care much about shutter speed, aperture value etc, I just take pictures, "
                                           "and enjoy looking at them afterwards. In other words, only few of my pictures are of "
                                           "fancy flowers in the right light, while most are pictures from holidays, parties, "
                                           "and other such events. How about you?"),
                                     i18n( "How are you using your camera?"),
                                     QStringList() << i18n("I'm mostly interested in techniques of photographing.")
                                     << i18n( "I take pictures to remember and enjoy given events (say a holiday).")
                                     << i18n( "I can't really answer, a bit of both I guess." ), this );

    new Survey::RadioButtonQuestion( QString::fromLatin1( "ImportExport" ), i18n("Import / Export"),
                                     i18n("KPhotoAlbum has the capability to export your images in a format, so annotations etc "
                                         "may be imported by another KPhotoAlbum user. "
                                         "(The feature is available in the File menu, and also offered during HTML export)" ),
                                     i18n("How often are you using Import/Export?"),
                                     QStringList() << i18n("Never I didn't know it existed")
                                     << i18n("Never, I don't know any other KPhotoAlbum users")
                                     << i18n("Seldomly")
                                     << i18n("From time to time")
                                     << i18n("Often"), this );

    new Survey::RadioButtonQuestion( QString::fromLatin1( "KIPI" ), i18n("KIPI"),
                                     i18n("KPhotoAlbum supports KIPI, a set of plug-ins shared among a number of imageing application."
                                         "All the features in the Plugins menu are from KIPI."),
                                     i18n("How often are you using KIPI features?"),
                                     QStringList() << i18n( "Never, I don't have a Plugins menu item" )
                                     << i18n("Never, I didn't find anything I need there" )
                                     << i18n("Seldomly" )
                                     << i18n("Once in a while" )
                                     << i18n("Often"), this );

    QStringList imageAppList;
    imageAppList << QString::fromLatin1( "Digikam" ) << QString::fromLatin1( "Gwenview" ) << QString::fromLatin1( "kuickshow" );

    new Survey::AlternativeQuestion( QString::fromLatin1( "OtherApps" ),
                                     i18n( "Other Image Applications" ),
                                     QString::null,
                                     i18n("Which other image applications are you using?"), imageAppList, 5,
                                     Survey::AlternativeQuestion::CheckBox, this );

    new Survey::TextQuestion( QString::fromLatin1( "Comment" ),
                              i18n("General Comments"),
                              i18n("<qt>If you have any comments that you would like to make, here would be a good place. "
                                   "comments like <i>This is really an awesome piece of software, how did I live without</i> "
                                   "is of course very welcome, but I'd also like to know if you see a feature missing that "
                                   "would prevent you from getting your best friend using KPhotoAlbum.</qt>"), this );

    txt = i18n("<qt><p>Thank you very much for your time, I hope you will continue using KPhotoAlbum for many years to come, "
               "and that future versions will fit your purpose even better than it does today.</p>"
               "Finally, allow me to ask you to consider giving a donation. "
               "See the Help->Donation menu on how to make a donation.</p>"
               "<p align=\"right\">Once again thanks for filling out this survey - Jesper</p></qt>");

    Survey::InfoPage* back = new Survey::InfoPage( QString::fromLatin1("KPhotoAlbum"), txt, Survey::InfoPage::Back, this );
    setBackPage( back );


}

QSize MySurvey::sizeHint() const
{
    QSize size = Survey::SurveyDialog::sizeHint();
    return QSize( QMAX( size.width(), 800 ), size.height() );
}

#include "MySurvey.moc"
