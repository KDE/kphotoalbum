// SPDX-FileCopyrightText: 2020-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Logging.h"
#include "ThumbnailCacheConverter.h"

#include <kpabase/SettingsData.h>
#include <kpabase/version.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QLocale>
#include <QLoggingCategory>
#include <QTextStream>
#include <QTimer>

using namespace KPAThumbnailTool;

void checkConflictingOptions(const QCommandLineParser &parser, const QCommandLineOption &opt1, const QCommandLineOption &opt2, QTextStream &err)
{
    if (parser.isSet(opt1) && parser.isSet(opt2)) {
        err << i18nc("@info:shell", "Conflicting commandline options: %1 and %2!\n", opt1.names().constFirst(), opt2.names().constFirst());
        exit(1);
    }
}

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("kphotoalbum");
    QCoreApplication app(argc, argv);

    KAboutData aboutData(
        QStringLiteral("kpa-thumbnailtool"), // component name
        i18n("KPhotoAlbum Thumbnail Tool"), // display name
        QStringLiteral(KPA_VERSION),
        i18n("Tool for inspecting and editing the KPhotoAlbum thumbnail cache"), // short description
        KAboutLicense::GPL,
        i18n("Copyright (C) 2020-2024 The KPhotoAlbum Development Team"), // copyright statement
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

    // initialize the commandline parser
    QCommandLineParser parser;
    parser.addPositionalArgument(QString::fromUtf8("imageDir"), i18nc("@info:shell", "The folder containing the .thumbnail folder."));
    QCommandLineOption infoOption { QString::fromUtf8("info"), i18nc("@info:shell", "Print information about thumbnail cache.") };
    parser.addOption(infoOption);
    QCommandLineOption convertV5ToV4Option { QString::fromUtf8("convertV5ToV4"), i18nc("@info:shell", "Convert thumbnailindex to format suitable for KPhotoAlbum >= 4.3.") };
    parser.addOption(convertV5ToV4Option);
    QCommandLineOption verifyOption { QString::fromUtf8("check-thumbnail-dimensions"), i18nc("@info:shell", "Check thumbnail cache for consistency of thumbnail dimensions.") };
    parser.addOption(verifyOption);
    QCommandLineOption fixOption { QString::fromUtf8("remove-broken"), i18nc("@info:shell", "Fix inconsistent thumbnails by removing them from the cache (requires --check-thumbnail-dimensions).") };
    parser.addOption(fixOption);
    QCommandLineOption vacuumOption { QString::fromUtf8("vacuum"), i18nc("@info:shell", "Remove unreferenced thumbnails from the thumbnail data files.") };
    parser.addOption(vacuumOption);
    QCommandLineOption quietOption { QString::fromUtf8("quiet"), i18nc("@info:shell", "Be less verbose.") };
    parser.addOption(quietOption);

    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);
    QTextStream console { stdout };
    QTextStream err { stderr };

    checkConflictingOptions(parser, convertV5ToV4Option, infoOption, err);
    checkConflictingOptions(parser, convertV5ToV4Option, verifyOption, err);

    const auto args = parser.positionalArguments();
    if (args.empty()) {
        err << i18nc("@info:shell", "Missing argument!\n");
        return 1;
    }
    const auto imageDir = QDir { args.first() };
    if (!imageDir.exists()) {
        err << i18nc("@info:shell", "%1 is not a folder!\n", args.first());
        return 1;
    }
    if (parser.isSet(convertV5ToV4Option)) {
        const QString indexFile = imageDir.absoluteFilePath(QString::fromUtf8(".thumbnails/thumbnailindex"));
        return convertV5ToV4Cache(indexFile, err);
    }

    int returnValue = 0;
    DB::DummyUIDelegate uiDelegate;
    Settings::SettingsData::setup();
    Settings::SettingsData::instance()->setImageDirectory(imageDir.path());
    Settings::SettingsData::instance()->setUiDelegate(&uiDelegate);
    const auto thumbnailDir = imageDir.absoluteFilePath(ImageManager::defaultThumbnailDirectory());
    ImageManager::ThumbnailCache cache { thumbnailDir };
    if (parser.isSet(infoOption)) {
        console << i18nc("@info:shell", "Thumbnail cache folder: %1\n", thumbnailDir);
        console << i18nc("@info:shell", "Thumbnail index file version: %1\n", cache.actualFileVersion());
        console << i18nc("@info:shell", "Maximum supported thumbnailindex file version: %1\n", cache.preferredFileVersion());
        console << i18nc("@info:shell", "Thumbnail storage dimensions: %1 pixels\n", cache.thumbnailSize());
        if (cache.actualFileVersion() < 5 && !parser.isSet(quietOption)) {
            console << i18nc("@info:shell", "Note: Thumbnail storage dimensions are defined in the configuration file prior to v5.\n");
        }
        console << i18nc("@info:shell", "Number of thumbnails: %1\n", cache.size());
        console.flush();
    }
    if (parser.isSet(verifyOption)) {
        const auto incorrectDimensions = cache.findIncorrectlySizedThumbnails();
        if (incorrectDimensions.isEmpty()) {
            console << i18nc("@info:shell", "No inconsistencies found.\n");
        } else {
            returnValue = 1;
            if (!parser.isSet(quietOption)) {
                console << i18nc("@info:shell This line is printed before a list of file names.", "The following thumbnails appear to have incorrect sizes:\n");
                for (const auto &filename : incorrectDimensions) {
                    console << filename.absolute() << "\n";
                }
            }
            if (parser.isSet(fixOption)) {
                cache.removeThumbnails(incorrectDimensions);
                cache.save();
                console << i18ncp("@info:shell",
                                  "Removed 1 inconsistent thumbnail from the database.\n",
                                  "Removed %1 inconsistent thumbnails from the database.\n", incorrectDimensions.size());
            }
        }
        console.flush();
    }
    if (parser.isSet(vacuumOption)) {
        cache.vacuum();
    }

    // immediately quit the event loop:
    QTimer::singleShot(0, &app, [&app, returnValue]() { app.exit(returnValue); });
    return QCoreApplication::exec();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
