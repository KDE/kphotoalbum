// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "FeatureDialog.h"

#include <kpabase/config-kpa-marble.h>
#include <kpabase/config-kpa-plugins.h>
#include <kpaexif/Database.h>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLayout>
#include <QList>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <phonon/backendcapabilities.h>

using namespace MainWindow;

FeatureDialog::FeatureDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Feature Status"));

    QTextBrowser *browser = new QTextBrowser(this);

    QString text = i18n("<h1>Overview</h1>"
                        "<p>Below you may see the list of compile- and runtime features KPhotoAlbum has, and their status:</p>"
                        "%1",
                        featureString());
    text += i18n("<h1>What can I do if I miss a feature?</h1>"

                 "<p>If you compiled KPhotoAlbum yourself, then please review the sections below to learn what to install "
                 "to get the feature in question. If on the other hand you installed KPhotoAlbum from a binary package, please tell "
                 "whoever made the package about this defect, eventually including the information from the section below.</p>"

                 "<p>In case you are missing a feature and you did not compile KPhotoAlbum yourself, please do consider doing so. "
                 "It really is not that hard. If you need help compiling KPhotoAlbum, feel free to ask on the "
                 "<a href=\"https://mail.kdab.com/mailman/listinfo/kphotoalbum\">KPhotoAlbum mailing list</a></p>"

                 "<p>The steps to compile KPhotoAlbum can be seen on <a href=\"https://community.kde.org/KPhotoAlbum/build_instructions\">"
                 "the KPhotoAlbum home page</a>. If you have never compiled a KDE application, then please ensure that "
                 "you have the developer packages installed, in most distributions they go under names like kdelibs<i>-devel</i></p>");

    text += i18n("<h1><a name=\"purpose\">Plugin support</a></h1>"
                 "<p>KPhotoAlbum supports the <em>Purpose</em> plugin system.</p>");

    text += i18n("<h1><a name=\"database\">SQLite database support</a></h1>"
                 "<p>KPhotoAlbum allows you to search using a certain number of Exif tags. For this KPhotoAlbum "
                 "needs an SQLite database. "
                 "In addition the Qt package for SQLite (e.g. qt-sql-sqlite) must be installed.</p>");

    text += i18n("<h1><a name=\"geomap\">Map view for geotagged images</a></h1>"
                 "<p>If KPhotoAlbum has been built with support for Marble, "
                 "KPhotoAlbum can show images with GPS information on a map."
                 "</p>");

    text += i18n("<h1><a name=\"video\">Video support</a></h1>"
                 "<p>KPhotoAlbum relies on Qt's Phonon architecture for displaying videos; this in turn relies on GStreamer.</p>");

    QStringList mimeTypes = supportedVideoMimeTypes();
    mimeTypes.sort();
    if (mimeTypes.isEmpty())
        text += i18n("<p>No video mime types found, which indicates that either Qt was compiled without phonon support, or there were missing codecs</p>");
    else
        text += i18n("<p>Phonon is capable of playing movies of these mime types:<ul><li>%1</li></ul></p>", mimeTypes.join(QString::fromLatin1("</li><li>")));

    text += i18n("<h1><a name=\"videoPreview\">Video thumbnail support</a></h1>"
                 "<p>KPhotoAlbum can use <tt>ffmpeg</tt> to extract thumbnails from videos. These thumbnails are used to preview "
                 "videos in the thumbnail viewer.</p>");

    text += i18n("<h1><a name=\"videoInfo\">Video metadata support</a></h1>"
                 "<p>KPhotoAlbum can use <tt>ffprobe</tt> to extract length information from videos."
                 "</p>"
                 "<p>Correct length information is necessary for correct rendering of video thumbnails.</p>");

    browser->setText(text);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(browser);
    this->setLayout(layout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

QSize FeatureDialog::sizeHint() const
{
    return QSize(800, 600);
}

bool MainWindow::FeatureDialog::hasPurposeSupport()
{
#ifdef KF5Purpose_FOUND
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasEXIV2DBSupport()
{
    return Exif::Database::isAvailable();
}

bool MainWindow::FeatureDialog::hasGeoMapSupport()
{
#ifdef HAVE_MARBLE
    return true;
#else
    return false;
#endif
}

QString FeatureDialog::ffmpegBinary()
{
    QString ffmpeg = QStandardPaths::findExecutable(QString::fromLatin1("ffmpeg"));
    return ffmpeg;
}

QString FeatureDialog::ffprobeBinary()
{
    QString ffprobe = QStandardPaths::findExecutable(QString::fromLatin1("ffprobe"));
    return ffprobe;
}

bool FeatureDialog::hasVideoThumbnailer()
{
    return !ffmpegBinary().isEmpty();
}

bool FeatureDialog::hasVideoProber()
{
    return !ffprobeBinary().isEmpty();
}

bool MainWindow::FeatureDialog::hasAllFeaturesAvailable()
{
    // Only answer those that are compile time tests, otherwise we will pay a penalty each time we start up.
    return hasPurposeSupport() && hasEXIV2DBSupport() && hasGeoMapSupport() && hasVideoThumbnailer() && hasVideoProber();
}

struct Data {
    Data() { }
    Data(const QString &title, const QString tag, bool featureFound)
        : title(title)
        , tag(tag)
        , featureFound(featureFound)
    {
    }
    QString title;
    QString tag;
    bool featureFound;
};

QString MainWindow::FeatureDialog::featureString()
{
    QList<Data> features;
    features << Data(i18n("Plug-ins available"), QString::fromLatin1("#purpose"), hasPurposeSupport());
    features << Data(i18n("SQLite database support (used for Exif searches)"), QString::fromLatin1("#database"), hasEXIV2DBSupport());
    features << Data(i18n("Map view for geotagged images."), QString::fromLatin1("#geomap"), hasGeoMapSupport());
    features << Data(i18n("Video support"), QString::fromLatin1("#video"), !supportedVideoMimeTypes().isEmpty());
    features << Data(i18n("Video thumbnail support"), QString::fromLatin1("#videoPreview"), hasVideoThumbnailer());
    features << Data(i18n("Video metadata support"), QString::fromLatin1("#videoInfo"), hasVideoProber());

    QString result = QString::fromLatin1("<p><table>");
    const QString red = QString::fromLatin1("<font color=\"red\">%1</font>");
    const QString yes = i18nc("Feature available", "Yes");
    const QString no = red.arg(i18nc("Feature not available", "No"));
    const QString formatString = QString::fromLatin1("<tr><td><a href=\"%1\">%2</a></td><td><b>%3</b></td></tr>");
    for (QList<Data>::ConstIterator featureIt = features.constBegin(); featureIt != features.constEnd(); ++featureIt) {
        result += formatString
                      .arg((*featureIt).tag)
                      .arg((*featureIt).title)
                      .arg((*featureIt).featureFound ? yes : no);
    }
    result += QString::fromLatin1("</table></p>");

    return result;
}

QStringList MainWindow::FeatureDialog::supportedVideoMimeTypes()
{
    return Phonon::BackendCapabilities::availableMimeTypes();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
