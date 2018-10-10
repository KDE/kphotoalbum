/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "DemoUtil.h"
#include "Util.h"
#include "Logging.h"

#include <MainWindow/Window.h>

#include <KJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

#include <KIO/DeleteJob>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

namespace {

void copyList( const QStringList& from, const QString& directoryTo )
{
    for( QStringList::ConstIterator it = from.constBegin(); it != from.constEnd(); ++it ) {
        const QString destFile = directoryTo + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            const bool ok = Utilities::copy( *it, destFile );
            if ( !ok ) {
                KMessageBox::error( nullptr, i18n("Unable to copy '%1' to '%2'.", *it , destFile ), i18n("Error Running Demo") );
                exit(-1);
            }
        }
    }
}
}

// vi:expandtab:tabstop=4 shiftwidth=4:

QString Utilities::setupDemo()
{
    const QString demoDir = QString::fromLatin1( "%1/kphotoalbum-demo-%2" ).arg(QDir::tempPath()).arg(QString::fromLocal8Bit( qgetenv( "LOGNAME" ) ));
    QFileInfo fi(demoDir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( demoDir );
        if ( !ok ) {
            KMessageBox::error( nullptr, i18n("Unable to create directory '%1' needed for demo.", demoDir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    // index.xml
    const QString demoDB = locateDataFile(QString::fromLatin1("demo/index.xml"));
    if ( demoDB.isEmpty() )
    {
        qCDebug(UtilitiesLog) << "No demo database in standard locations:" << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        exit(-1);
    }
    const QString configFile = demoDir + QString::fromLatin1( "/index.xml" );
    copy(demoDB, configFile);

    // Images
    const QStringList kpaDemoDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                QString::fromLatin1("demo"),
                QStandardPaths::LocateDirectory);
    QStringList images;
    Q_FOREACH(const QString &dir, kpaDemoDirs)
    {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.jpg") << QStringLiteral("*.avi"));
        while (it.hasNext()) {
            images.append(it.next());
        }
    }
    copyList( images, demoDir );

    // CategoryImages
    QString catDir = demoDir + QString::fromLatin1("/CategoryImages");
    fi = QFileInfo(catDir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( catDir  );
        if ( !ok ) {
            KMessageBox::error( nullptr, i18n("Unable to create directory '%1' needed for demo.", catDir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    const QStringList kpaDemoCatDirs = QStandardPaths::locateAll(
                QStandardPaths::DataLocation,
                QString::fromLatin1("demo/CategoryImages"),
                QStandardPaths::LocateDirectory);
    QStringList catImages;
    Q_FOREACH(const QString &dir, kpaDemoCatDirs)
    {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.jpg"));
        while (it.hasNext()) {
            catImages.append(it.next());
        }
    }
    copyList( catImages, catDir );

    return configFile;
}

void Utilities::deleteDemo()
{
    QString dir = QString::fromLatin1( "%1/kphotoalbum-demo-%2" ).arg(QDir::tempPath()).arg(QString::fromLocal8Bit( qgetenv( "LOGNAME" ) ) );
    QUrl demoUrl = QUrl::fromLocalFile( dir );
    KJob *delDemoJob = KIO::del( demoUrl );
    KJobWidgets::setWindow( delDemoJob, MainWindow::Window::theMainWindow());
    delDemoJob->exec();
}
