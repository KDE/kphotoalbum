#include "mysurvey.h"
#include <textquestion.h>
#include "survey-count.h"
#include <infopage.h>
#include <klocale.h>
#include <alternativequestion.h>
MySurvey::MySurvey( QWidget* parent, const char* name )
    :Survey::SurveyDialog( parent, name )
{
    setSurveyVersion( 1, 1 );
    setReceiver( QString::fromLatin1( "blackie@kde.org" ) );

    QStringList yesNoList;
    yesNoList << i18n("Yes") << i18n("No");

    QString txt
        = i18n("<qt><p>This is the KimDaBa survey, its intention is to make KimDaBa fit the need of <i>you</i> - my valued user.</p>"
               "I have spent most of my sparetime on KimDaBa the last two and a half year, so I'd be really happy if you "
               "could spent the next five to ten minute telling me what you think about it, and giving me some "
               "feedback on which part of the application you are using.</p>"
               "<p>At any time you may quit the survey and get back to it later, it will remember your answers</p>"
               "<p align=\"right\">Thanks in advance - Jesper</p></qt>");

    Survey::InfoPage* front = new Survey::InfoPage( QString::fromLatin1("KimDaBa"), txt, Survey::InfoPage::Front, this );
    setFrontPage( front );

    new Survey::RadioButtonQuestion( QString::fromLatin1( "HowLongHaveYouUsedKimDaBa" ),
                                     i18n("Length of Usage"),
                                     QString::null,
                                     i18n("How long have you used KimDaBa?"),
                                     QStringList() << i18n("< 1 Month") << i18n("1-6 Months") << i18n("6-12 Months")
                                     << i18n("1-2 Year" ) << i18n("> 2 Years"), this );

    new SurveyCountQuestion( QString::fromLatin1( "ImageCount" ),
                             i18n("Image Count"),
                             this );

    new Survey::RadioButtonQuestion( QString::fromLatin1("StartUpTime"),
                                     i18n("Start Up Time"),
                                     i18n("It is always possible to get KimDaBa to start faster, but doing so "
                                          "would e.g. mean that KimDaBa's backend should be changed from an XML file "
                                          "to a database. Doing so would take time, time that otherwise could be "
                                          "spent on implementing other features in KimDaBa."),
                                     i18n("Would you prefer that time was spent on making KimDaBa faster?"), yesNoList, this );

    QStringList categories;
    categories << i18n("Persons") << i18n("Locations") << i18n("Keywords");
    new Survey::AlternativeQuestion( QString::fromLatin1( "CategoriesUsed" ),
                                     i18n("Categories Used"),
                                     QString::null,
                                     i18n("Which categories are you using?"),
                                     categories, 5, Survey::AlternativeQuestion::CheckBox, this );

    new Survey::RadioButtonQuestion( QString::fromLatin1( "MemberGroups" ), i18n("Member Groups"),
                                     i18n("One of the main features of KimDaBa is that it is possible to create member groups. "
                                          "Using this feature, you can specify that Las Vegas is in Nevada, which is in USA, "
                                          "which is on the earth and so on. Whenever you look for images from Nevada, USA, or the earth, "
                                          "you will also see the images from Las Vegas."),
                                     i18n("Are you using member groups?"), yesNoList, this );

    QStringList offLineList;
    offLineList << i18n("Yes") << i18n("No but I expect to have within the next year") << i18n("No and I don't expect within the next year" );
    new Survey::RadioButtonQuestion( QString::fromLatin1( "OffLineMode" ), i18n("Offline Mode"),
                                     i18n("<p>If you have more images than can be stored on your disk, KimDaBa allows you to store some of them "
                                          "on offline medias like cd's or dvd's. If an image is not available, KimDaBa will show it with the corner cut off "
                                          "in the thumbnail viewer.</p>"
                                          "<p>Currently you need to restart KimDaBa when an image has become available (like a dvd has been mounted), and you need "
                                          "to do quite a bit of symlinks tricks to always show the thumbnails even when the real images is not available.</p>" ),
                                     i18n("Are you having more images than fits on your disk?"), offLineList, this );
    QStringList imageAppList;
    imageAppList << QString::fromLatin1( "Digikam" ) << QString::fromLatin1( "Gvenview" ) << QString::fromLatin1( "kuickshow" );

    new Survey::AlternativeQuestion( QString::fromLatin1( "OtherApps" ),
                                     i18n( "Other Image Applications" ),
                                     QString::null,
                                     i18n("Which other image applications are you using?"), imageAppList, 5,
                                     Survey::AlternativeQuestion::CheckBox, this );

    QStringList kimdabaUsage;
    kimdabaUsage << i18n("Private albums") << i18n("Professional");

    new Survey::AlternativeQuestion( QString::fromLatin1( "WhatAreYouUsingKimDaBaFor" ),
                                     i18n( "For what are you using KimDaBa?"),
                                     QString::null, i18n( "For what are you using KimDaBa?"),
                                     kimdabaUsage, 2, Survey::AlternativeQuestion::CheckBox, this );

    new Survey::TextQuestion( QString::fromLatin1( "Comment" ),
                              i18n("General Comments"),
                              i18n("<qt>If you have any comments that you would like to make, here would be a good place. "
                                   "comments like <i>This is really an awesome piece of software, how did I live without</i> "
                                   "is of course very welcome, but I'd also like to know if you see a feature missing that "
                                   "would prevent you from getting your best friend using KimDaBa.</qt>"), this );

    txt = i18n("<qt><p>Thank you very much for your time, I hope you will continue using KimDaBa for many years to come, "
               "and that future versions will fit your purpose even better than it does today.</p>"
               "Finally, allow me to ask you to consider giving a donation. My own digital camera is starting to get old "
               "so I hope that all of you would show your appreciation of my work, by helping me buy a new one, that will "
               "ensure that I enjoy taking pictures - and sorting them for many years to come. "
               "See the Help->Donation menu on how to make a donation.</p>"
               "<p align=\"right\">Once again thanks for filling out this survey - Jesper</p></qt>");

    Survey::InfoPage* back = new Survey::InfoPage( QString::fromLatin1("KimDaBa"), txt, Survey::InfoPage::Back, this );
    setBackPage( back );


}

QSize MySurvey::sizeHint() const
{
    QSize size = Survey::SurveyDialog::sizeHint();
    return QSize( QMAX( size.width(), 800 ), size.height() );
}

#include "mysurvey.moc"
