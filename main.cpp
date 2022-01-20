// SPDX-FileCopyrightText: 2003 Lukáš Tinkl <lukas@kde.org>
// SPDX-FileCopyrightText: 2003 Simon Hausmann <hausmann@kde.org>
// SPDX-FileCopyrightText: 2003 Stephan Binner <binner@kde.org>
// SPDX-FileCopyrightText: 2003-2007, 2009-2014 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2005, 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2006-2008 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Chusslove Illich <caslav.ilic@gmx.net>
// SPDX-FileCopyrightText: 2007, 2011 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2009 Andrew Coles <andrew.i.coles@googlemail.com>
// SPDX-FileCopyrightText: 2009 Christoph Feck <cfeck@kde.org>
// SPDX-FileCopyrightText: 2010, 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012-2014, 2016, 2018-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2018 Antoni Bella Pérez <antonibella5@yahoo.com>
// SPDX-FileCopyrightText: 2018, 2020 Tobias Leupold <tl at stonemx dot de>
// SPDX-FileCopyrightText: 2019 Alexander Potashev <aspotashev@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <MainWindow/Options.h>
#include <MainWindow/SplashScreen.h>
#include <MainWindow/Window.h>
#ifdef KPA_ENABLE_REMOTECONTROL
#include <RemoteControl/RemoteInterface.h>
#endif
#include <kpabase/SettingsData.h>
#include <kpabase/version.h>

#include <KAboutData>
#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <Kdelibs4ConfigMigrator>
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QLocale>
#include <QLoggingCategory>
#include <QTemporaryFile>

Q_DECLARE_LOGGING_CATEGORY(MainLog)
Q_LOGGING_CATEGORY(MainLog, "kphotoalbum", QtWarningMsg)

namespace
{

const auto STYLE = QStringLiteral(
    "Viewer--TaggedArea { border: none; background-color: none; }"
    "Viewer--TaggedArea:hover, Viewer--TaggedArea[selected=\"true\"] {"
    " border: 1px solid rgb(0,255,0,99); background-color: rgb(255,255,255,30);"
    " }"
    "Viewer--TaggedArea[highlighted=\"true\"]{ border: 1px solid rgb(255,128,0,99); background-color: rgb(255,255,255,30); }"
    "AnnotationDialog--ResizableFrame { color: rgb(255,0,0); }"
    "AnnotationDialog--ResizableFrame:hover { background-color: rgb(255,255,255,30); }"
    "AnnotationDialog--ResizableFrame[associated=true] { color: rgb(0,255,0); }");
}
void migrateKDE4Config()
{
    Kdelibs4ConfigMigrator migrator(QStringLiteral("kphotoalbum")); // the same name defined in the aboutData
    migrator.setConfigFiles(QStringList() << QStringLiteral("kphotoalbumrc"));
    migrator.setUiFiles(QStringList() << QStringLiteral("kphotoalbumui.rc"));
    if (migrator.migrate()) {
        KConfigGroup unnamedConfig = KSharedConfig::openConfig()->group(QString());
        if (unnamedConfig.hasKey(QStringLiteral("configfile"))) {
            // rename config file entry on update
            KConfigGroup generalConfig = KSharedConfig::openConfig()->group(QStringLiteral("General"));
            generalConfig.writeEntry(QStringLiteral("imageDBFile"),
                                     unnamedConfig.readEntry(QStringLiteral("configfile")));
            unnamedConfig.deleteEntry(QStringLiteral("configfile"));
            qCWarning(MainLog) << "Renamed config entry configfile to General.imageDBFile.";
        }
    }
}

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("kphotoalbum");
    QApplication app(argc, argv);
    migrateKDE4Config();

    KAboutData aboutData(
        QStringLiteral("kphotoalbum"), // component name
        i18n("KPhotoAlbum"), // display name
        QStringLiteral(KPA_VERSION),
        i18n("KDE Photo Album"), // short description
        KAboutLicense::GPL_V3,
        i18n("Copyright (C) 2003-2023 The KPhotoAlbum Development Team"), // copyright statement
        QString(), // other text
        QStringLiteral("https://www.kphotoalbum.org") // homepage
    );
    aboutData.setOrganizationDomain("kde.org");
    // maintainer is expected to be the first entry
    // Note: I like to sort by name, grouped by active/inactive;
    //       Jesper gets ranked with the active authors for obvious reasons
    aboutData.addAuthor(i18n("Johannes Zarl-Zierl"), i18n("Development, Maintainer"), QStringLiteral("johannes@zarl-zierl.at"));
    aboutData.addAuthor(i18n("Robert Krawitz"), i18n("Development, Optimization"), QStringLiteral("rlk@alum.mit.edu"));
    aboutData.addAuthor(i18n("Tobias Leupold"), i18n("Development, Releases, Website"), QStringLiteral("tl@stonemx.de"));
    aboutData.addAuthor(i18n("Jesper K. Pedersen"), i18n("Former Maintainer, Project Creator"), QStringLiteral("blackie@kde.org"));
    // not currently active:
    aboutData.addAuthor(i18n("Hassan Ibraheem"), QString(), QStringLiteral("hasan.ibraheem@gmail.com"));
    aboutData.addAuthor(i18n("Jan Kundr&aacute;t"), QString(), QStringLiteral("jkt@gentoo.org"));
    aboutData.addAuthor(i18n("Andreas Neustifter"), QString(), QStringLiteral("andreas.neustifter@gmail.com"));
    aboutData.addAuthor(i18n("Tuomas Suutari"), QString(), QStringLiteral("thsuut@utu.fi"));
    aboutData.addAuthor(i18n("Miika Turkia"), QString(), QStringLiteral("miika.turkia@gmail.com"));
    aboutData.addAuthor(i18n("Henner Zeller"), QString(), QStringLiteral("h.zeller@acm.org"));

    // sort alphabetically:
    aboutData.addCredit(i18n("Marco Caldarelli"), i18n("Patch for making it possible to reread Exif info using a nice dialog."), QStringLiteral("caldarel@yahoo.it"));
    aboutData.addCredit(i18n("Jean-Michel FAYARD"), i18n("(.) Patch with folder info made available through the browser. (.) Patch for adding a check box for \"and/or\" searches in the search page."), QStringLiteral("jmfayard@gmail.com"));
    aboutData.addCredit(i18n("Matthias Füssel"), i18n("Geo Position page in browser and various bug fixes"), QStringLiteral("matthias.fuessel@gmx.net"));
    aboutData.addCredit(i18n("Wes Hardaker"), i18n("Some very useful features to improve workflow"), QStringLiteral("kpa@capturedonearth.com"));
    aboutData.addCredit(i18n("Reimar Imhof"), i18n("Patch to sort items in option listboxes"), QStringLiteral("Reimar.Imhof@netCologne.de"));
    aboutData.addCredit(i18n("Christoph Moseler"), i18n("Numerous patches for lots of bugs plus patches for a few new features"), QStringLiteral("forums@moseler.net"));
    aboutData.addCredit(i18n("Teemu Rytilahti"),
                        i18n("Theme support for HTML generation"), QStringLiteral("teemu.rytilahti@kde-fi.org"));
    aboutData.addCredit(i18n("Thomas Schwarzgruber"), i18n("Patch to sort images in the thumbnail view, plus reading time info out of Exif images for existing images"), QStringLiteral("possebaer@gmx.at"));
    aboutData.addCredit(i18n("Clytie Siddall"), i18n("Tremendous help with the English text in the application."), QStringLiteral("clytie@riverland.net.au"));
    aboutData.addCredit(i18n("Will Stephenson"), i18n("Developing an Icon for KPhotoAlbum"), QStringLiteral("will@stevello.free-online.co.uk"));
    aboutData.addCredit(i18n("Marcel Wiesweg"), i18n("Patch which speed up loading of thumbnails plus preview in image property dialog."), QStringLiteral("marcel.wiesweg@gmx.de"));

    // initialize the commandline parser
    QCommandLineParser *parser = MainWindow::Options::the()->parser();

    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(parser);

    parser->process(app);
    aboutData.processCommandLine(parser);

    const QString schemePath = KSharedConfig::openConfig()->group("General").readEntry(QStringLiteral("colorScheme"), QString());
    qCDebug(MainLog) << "Loading color scheme from " << (schemePath.isEmpty() ? QStringLiteral("system default") : schemePath);
    app.setPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(schemePath)));
    if (app.styleSheet().isEmpty())
        app.setStyleSheet(STYLE);
    app.setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles);

    new MainWindow::SplashScreen();

    // a KXmlGuiWindow per-default is created with the Qt::WA_DeleteOnClose attribute set
    // -> don't delete the view directly!
    MainWindow::Window *view = nullptr;
    try {
        view = new MainWindow::Window(nullptr);
    } catch (int retVal) {
        // MainWindow ctor throws if no config is loaded
        return retVal;
    }

#ifdef KPA_ENABLE_REMOTECONTROL
    if (MainWindow::Options::the()->hideInitialWindow())
        view->showMinimized();
    else
#endif
    {
        const auto mainWindowGeometry = Settings::SettingsData::instance()->windowGeometry(Settings::MainWindow);
        if (mainWindowGeometry.isValid())
            view->setGeometry(mainWindowGeometry);
        else
            view->showMaximized();
    }

#ifdef KPA_ENABLE_REMOTECONTROL
    (void)RemoteControl::RemoteInterface::instance();
//    RemoteControl::RemoteInterface::instance().listen(QHostAddress::Any);
#endif

    int code = QApplication::exec();
    return code;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
