/* Copyright (C) 2010 Miika Turkia <miika.turkia@gmail.com>

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

#include "AutoStackImages.h"
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <klocale.h>
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/ImageDate.h"
#include "DB/FileInfo.h"
#include "MainWindow/Window.h"
#include <qapplication.h>
#include <qeventloop.h>
#include "Utilities/ShowBusyCursor.h"
#include "Utilities/Util.h"
#include <QGroupBox>
#include <QTextEdit>
#include <KProgressDialog>
#include <kdebug.h>

using namespace MainWindow;

AutoStackImages::AutoStackImages( QWidget* parent, const DB::IdList& list )
    :KDialog( parent ), _list( list )
{
    setWindowTitle( i18n("Automatically Stack Images" ) );
    setButtons( Cancel | Ok );

    QWidget* top = new QWidget;
    setMainWidget( top );
    QVBoxLayout* lay1 = new QVBoxLayout( top );

    QWidget* containerMd5 = new QWidget( this );
    lay1->addWidget( containerMd5 );
    QHBoxLayout* hlayMd5 = new QHBoxLayout( containerMd5 );

    _matchingMD5 = new QCheckBox( i18n( "Stack images with identical MD5 sum") );
    _matchingMD5->setChecked( false );
    hlayMd5->addWidget( _matchingMD5 );

    QWidget* containerContinuous = new QWidget( this );
    lay1->addWidget( containerContinuous );
    QHBoxLayout* hlayContinuous = new QHBoxLayout( containerContinuous );

    //FIXME: This is hard to translate because of the split sentence. It is better
    //to use a single sentence here like "Stack images that are (were?) shot
    //within this time:" and use the spin method setSuffix() to set the "seconds".
    //Also: Would minutes not be a more sane time unit here? (schwarzer)
    _continuousShooting = new QCheckBox( i18nc( "The whole sentence should read: *Stack images that are shot within x seconds of each other*. So images that are shot in one burst are automatically stacked together. (This sentence is before the x.)", "Stack images that are shot within" ) );
    _continuousShooting->setChecked( true );
    hlayContinuous->addWidget( _continuousShooting );

    _continuousThreshold = new QSpinBox;
    _continuousThreshold->setRange( 1, 999 );
    _continuousThreshold->setSingleStep( 1 );
    _continuousThreshold->setValue( 2 );
    hlayContinuous->addWidget( _continuousThreshold );

    QLabel* sec = new QLabel( i18nc( "The whole sentence should read: *Stack images that are shot within x seconds of each other*. (This being the text after x.)", "seconds" ), containerContinuous );
    hlayContinuous->addWidget( sec );

    QGroupBox* grpOptions = new QGroupBox( i18n("AutoStacking Options") );
    QVBoxLayout* grpLayOptions = new QVBoxLayout( grpOptions );
    lay1->addWidget( grpOptions );

    _autostackDefault = new QRadioButton( i18n( "Include matching image to appropriate stack (if one exists)") );
    _autostackDefault->setChecked( true );
    grpLayOptions->addWidget( _autostackDefault );

    _autostackUnstack = new QRadioButton( i18n( "Unstack images from their current stack and create new one for the matches") );
    _autostackUnstack->setChecked( false );
    grpLayOptions->addWidget( _autostackUnstack );

    _autostackSkip = new QRadioButton( i18n( "Skip images that are already in a stack") );
    _autostackSkip->setChecked( false );
    grpLayOptions->addWidget( _autostackSkip );
}

/*
 * This function searches for images with matching MD5 sums
 * Matches are automatically stacked
 */

void AutoStackImages::matchingMD5( DB::IdList &toBeShown )
{
    QMap< DB::MD5, QList<QString> > tostack;
    QList< QString > showIfStacked;

    // Stacking all images that have the same MD5 sum
    // First make a map of MD5 sums with corresponding images
    Q_FOREACH(const DB::ImageInfoPtr info, _list.fetchInfos()) {
        QString fileName = info->fileName(DB::AbsolutePath);
        DB::MD5 sum = info->MD5Sum();
        if ( DB::ImageDB::instance()->md5Map()->contains( sum ) ) {
            if (tostack[sum].isEmpty())
                tostack.insert(sum, (QStringList) fileName);
            else
                tostack[sum].append(fileName);
        }
    }

    // Then add images to stack (depending on configuration options)
    for( QMap<DB::MD5, QList<QString> >::ConstIterator it = tostack.constBegin(); it != tostack.constEnd(); ++it ) {
        if ( tostack[it.key()].count() > 1 ) {
            DB::IdList stack = DB::IdList();
            for ( int i = 0; i < tostack[it.key()].count(); ++i ) {
                if ( !DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( tostack[it.key()][i] ) ).isEmpty() ) {
                    if ( _autostackUnstack->isChecked() )
                        DB::ImageDB::instance()->unstack( (DB::IdList) DB::ImageDB::instance()->ID_FOR_FILE( tostack[it.key()][i] ) );
                    else if ( _autostackSkip->isChecked() )
                        continue;
                }

                showIfStacked.append( tostack[it.key()][i] );
                stack.append( DB::ImageDB::instance()->ID_FOR_FILE( tostack[it.key()][i] ) );
            }
            if ( stack.size() > 1 ) {
                
                foreach( const QString& a, showIfStacked ) {

                    if ( !DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( a ) ).isEmpty() )
                        foreach( DB::Id b, DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( a ) ) )
                            toBeShown.append( b );
                    else
                        toBeShown.append( DB::ImageDB::instance()->ID_FOR_FILE( a ) );
                }
                DB::ImageDB::instance()->stack( stack );
            }
            showIfStacked.clear();
        }
    }
}

/*
 * This function searches for images that are shot within specified time frame
 */

void AutoStackImages::continuousShooting(DB::IdList &toBeShown )
{
    DB::ImageInfoPtr prev;
    Q_FOREACH(const DB::ImageInfoPtr info, _list.fetchInfos()) {
        // Skipping images that do not have exact time stamp
        if ( info->date().start() != info->date().end() )
            continue;
        if ( !prev.isNull() && ( prev->date().start().secsTo( info->date().start() ) < _continuousThreshold->value() ) ) {
            DB::IdList stack = DB::IdList();

            if ( !DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( prev->fileName( DB::AbsolutePath ) ) ).isEmpty() ) {
                if ( _autostackUnstack->isChecked() )
                    DB::ImageDB::instance()->unstack( (DB::IdList) DB::ImageDB::instance()->ID_FOR_FILE( prev->fileName( DB::AbsolutePath ) ) );
                else if ( _autostackSkip->isChecked() )
                    continue;
            }
            
            if ( !DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( info->fileName( DB::AbsolutePath ) ) ).isEmpty() ) {
                if ( _autostackUnstack->isChecked() )
                    DB::ImageDB::instance()->unstack( (DB::IdList) DB::ImageDB::instance()->ID_FOR_FILE( info->fileName( DB::AbsolutePath ) ) );
                else if ( _autostackSkip->isChecked() )
                    continue;
            }

            stack.append( DB::ImageDB::instance()->ID_FOR_FILE( prev->fileName(DB::AbsolutePath) ) );
            stack.append( DB::ImageDB::instance()->ID_FOR_FILE( info->fileName(DB::AbsolutePath) ) );
            if ( !toBeShown.isEmpty() ) {
                if ( toBeShown.at( toBeShown.size() - 1 ).fetchInfo()->fileName( DB::RelativeToImageRoot ) != prev->fileName( DB::RelativeToImageRoot ) )
                     toBeShown.append( DB::ImageDB::instance()->ID_FOR_FILE( prev->fileName( DB::AbsolutePath ) ) );
            } else {
                // if this is first insert, we have to include also the stacked images from previuous image
                if ( !DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( info->fileName( DB::AbsolutePath ) ) ).isEmpty() )
                    foreach( DB::Id a, DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( prev->fileName( DB::AbsolutePath ) ) ) )
                        toBeShown.append( a );
                else
                    toBeShown.append( DB::ImageDB::instance()->ID_FOR_FILE( prev->fileName( DB::AbsolutePath ) ) );
            }
            // Inserting stacked images from the current image
            if ( !DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( info->fileName( DB::AbsolutePath ) ) ).isEmpty() )
                foreach( DB::Id a, DB::ImageDB::instance()->getStackFor( DB::ImageDB::instance()->ID_FOR_FILE( info->fileName( DB::AbsolutePath ) ) ) )
                    toBeShown.append( a );
            else
                toBeShown.append( DB::ImageDB::instance()->ID_FOR_FILE( info->fileName( DB::AbsolutePath ) ) );
            DB::ImageDB::instance()->stack( stack );
        }

        prev = info;
    }
}

void AutoStackImages::accept()
{
    KDialog::accept();
    Utilities::ShowBusyCursor dummy;
    DB::IdList toBeShown;

    if ( _matchingMD5->isChecked() )
        matchingMD5( toBeShown );
    if ( _continuousShooting->isChecked() )
        continuousShooting( toBeShown );

    MainWindow::Window::theMainWindow()->showThumbNails( toBeShown );
}

#include "AutoStackImages.moc"
