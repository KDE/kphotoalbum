/* Copyright (C) 2010-2018 Miika Turkia <miika.turkia@gmail.com>

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

#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <DB/FileInfo.h>
#include <DB/ImageDate.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/MD5Map.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>
#include <Utilities/ShowBusyCursor.h>
#include <Utilities/Util.h>

using namespace MainWindow;

AutoStackImages::AutoStackImages( QWidget* parent, const DB::FileNameList& list )
    :QDialog( parent ), m_list( list )
{
    setWindowTitle( i18nc("@title:window", "Automatically Stack Images" ) );

    QWidget* top = new QWidget;
    QVBoxLayout* lay1 = new QVBoxLayout( top );
    setLayout(lay1);

    QWidget* containerMd5 = new QWidget( this );
    lay1->addWidget( containerMd5 );
    QHBoxLayout* hlayMd5 = new QHBoxLayout( containerMd5 );

    m_matchingMD5 = new QCheckBox( i18n( "Stack images with identical MD5 sum") );
    m_matchingMD5->setChecked( false );
    hlayMd5->addWidget( m_matchingMD5 );

    QWidget* containerFile = new QWidget( this );
    lay1->addWidget( containerFile );
    QHBoxLayout* hlayFile = new QHBoxLayout( containerFile );

    m_matchingFile = new QCheckBox( i18n( "Stack images based on file version detection") );
    m_matchingFile->setChecked( true );
    hlayFile->addWidget( m_matchingFile );
 
    m_origTop = new QCheckBox( i18n( "Original to top") );
    m_origTop ->setChecked( false );
    hlayFile->addWidget( m_origTop );

    QWidget* containerContinuous = new QWidget( this );
    lay1->addWidget( containerContinuous );
    QHBoxLayout* hlayContinuous = new QHBoxLayout( containerContinuous );

    //FIXME: This is hard to translate because of the split sentence. It is better
    //to use a single sentence here like "Stack images that are (were?) shot
    //within this time:" and use the spin method setSuffix() to set the "seconds".
    //Also: Would minutes not be a more sane time unit here? (schwarzer)
    m_continuousShooting = new QCheckBox( i18nc( "The whole sentence should read: *Stack images that are shot within x seconds of each other*. So images that are shot in one burst are automatically stacked together. (This sentence is before the x.)", "Stack images that are shot within" ) );
    m_continuousShooting->setChecked( false );
    hlayContinuous->addWidget( m_continuousShooting );

    m_continuousThreshold = new QSpinBox;
    m_continuousThreshold->setRange( 1, 999 );
    m_continuousThreshold->setSingleStep( 1 );
    m_continuousThreshold->setValue( 2 );
    hlayContinuous->addWidget( m_continuousThreshold );

    QLabel* sec = new QLabel( i18nc( "The whole sentence should read: *Stack images that are shot within x seconds of each other*. (This being the text after x.)", "seconds" ), containerContinuous );
    hlayContinuous->addWidget( sec );

    QGroupBox* grpOptions = new QGroupBox( i18n("AutoStacking Options") );
    QVBoxLayout* grpLayOptions = new QVBoxLayout( grpOptions );
    lay1->addWidget( grpOptions );

    m_autostackDefault = new QRadioButton( i18n( "Include matching image to appropriate stack (if one exists)") );
    m_autostackDefault->setChecked( true );
    grpLayOptions->addWidget( m_autostackDefault );

    m_autostackUnstack = new QRadioButton( i18n( "Unstack images from their current stack and create new one for the matches") );
    m_autostackUnstack->setChecked( false );
    grpLayOptions->addWidget( m_autostackUnstack );

    m_autostackSkip = new QRadioButton( i18n( "Skip images that are already in a stack") );
    m_autostackSkip->setChecked( false );
    grpLayOptions->addWidget( m_autostackSkip );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AutoStackImages::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AutoStackImages::reject);
    lay1->addWidget(buttonBox);
}

/*
 * This function searches for images with matching MD5 sums
 * Matches are automatically stacked
 */

void AutoStackImages::matchingMD5( DB::FileNameList& toBeShown )
{
    QMap< DB::MD5, DB::FileNameList > tostack;
    DB::FileNameList showIfStacked;

    // Stacking all images that have the same MD5 sum
    // First make a map of MD5 sums with corresponding images
    Q_FOREACH(const DB::FileName& fileName, m_list) {
        DB::MD5 sum = fileName.info()->MD5Sum();
        if ( DB::ImageDB::instance()->md5Map()->contains( sum ) ) {
            if (tostack[sum].isEmpty())
                tostack.insert(sum, DB::FileNameList() << fileName);
            else
                tostack[sum].append(fileName);
        }
    }

    // Then add images to stack (depending on configuration options)
    for( QMap<DB::MD5, DB::FileNameList >::ConstIterator it = tostack.constBegin(); it != tostack.constEnd(); ++it ) {
        if ( tostack[it.key()].count() > 1 ) {
            DB::FileNameList stack;
            for ( int i = 0; i < tostack[it.key()].count(); ++i ) {
                if ( !DB::ImageDB::instance()->getStackFor( tostack[it.key()][i]).isEmpty() ) {
                    if ( m_autostackUnstack->isChecked() )
                        DB::ImageDB::instance()->unstack( DB::FileNameList() << tostack[it.key()][i]);
                    else if ( m_autostackSkip->isChecked() )
                        continue;
                }

                showIfStacked.append( tostack[it.key()][i] );
                stack.append( tostack[it.key()][i]);
            }
            if ( stack.size() > 1 ) {

                Q_FOREACH( const DB::FileName& a, showIfStacked ) {
                    if ( !DB::ImageDB::instance()->getStackFor(a).isEmpty() )
                        Q_FOREACH( const DB::FileName& b, DB::ImageDB::instance()->getStackFor(a))
                            toBeShown.append( b );
                    else
                        toBeShown.append(a);
                }
                DB::ImageDB::instance()->stack(stack);
            }
            showIfStacked.clear();
        }
    }
}

/*
 * This function searches for images based on file version detection configuration.
 * Images that are detected to be versions of same file are stacked together.
 */

void AutoStackImages::matchingFile( DB::FileNameList& toBeShown )
{
    QMap< DB::MD5, DB::FileNameList > tostack;
    DB::FileNameList showIfStacked;
    QString modifiedFileCompString;
    QRegExp modifiedFileComponent;
    QStringList originalFileComponents;
 
    modifiedFileCompString = Settings::SettingsData::instance()->modifiedFileComponent();
    modifiedFileComponent = QRegExp( modifiedFileCompString );

    originalFileComponents << Settings::SettingsData::instance()->originalFileComponent();
    originalFileComponents = originalFileComponents.at( 0 ).split( QString::fromLatin1(";") );

    // Stacking all images based on file version detection
    // First round prepares the stacking
    Q_FOREACH( const DB::FileName& fileName, m_list ) {
        if ( modifiedFileCompString.length() >= 0 &&
            fileName.relative().contains( modifiedFileComponent ) ) {

            for( QStringList::const_iterator it = originalFileComponents.constBegin();
                 it != originalFileComponents.constEnd(); ++it ) {
                QString tmp = fileName.relative();
                tmp.replace( modifiedFileComponent, ( *it ));
                DB::FileName originalFileName = DB::FileName::fromRelativePath( tmp );

                if ( originalFileName != fileName && m_list.contains( originalFileName ) ) {
                    DB::MD5 sum = originalFileName.info()->MD5Sum();
                    if ( tostack[sum].isEmpty() ) {
                        if ( m_origTop->isChecked() ) {
                            tostack.insert( sum, DB::FileNameList() << originalFileName );
                            tostack[sum].append( fileName );
                        } else {
                            tostack.insert( sum, DB::FileNameList() << fileName );
                            tostack[sum].append( originalFileName );
                        }
                    } else
                        tostack[sum].append(fileName);
                    break;
                }
            }
        }
    }
    
    // Then add images to stack (depending on configuration options)
    for( QMap<DB::MD5, DB::FileNameList >::ConstIterator it = tostack.constBegin(); it != tostack.constEnd(); ++it ) {
        if ( tostack[it.key()].count() > 1 ) {
            DB::FileNameList stack;
            for ( int i = 0; i < tostack[it.key()].count(); ++i ) {
                if ( !DB::ImageDB::instance()->getStackFor( tostack[it.key()][i]).isEmpty() ) {
                    if ( m_autostackUnstack->isChecked() )
                        DB::ImageDB::instance()->unstack( DB::FileNameList() << tostack[it.key()][i]);
                    else if ( m_autostackSkip->isChecked() )
                        continue;
                }

                showIfStacked.append( tostack[it.key()][i] );
                stack.append( tostack[it.key()][i]);
            }
            if ( stack.size() > 1 ) {

                Q_FOREACH( const DB::FileName& a, showIfStacked ) {
                    if ( !DB::ImageDB::instance()->getStackFor(a).isEmpty() )
                        Q_FOREACH( const DB::FileName& b, DB::ImageDB::instance()->getStackFor(a))
                            toBeShown.append( b );
                    else
                        toBeShown.append(a);
                }
                DB::ImageDB::instance()->stack(stack);
            }
            showIfStacked.clear();
        }
    }
}


/*
 * This function searches for images that are shot within specified time frame
 */

void AutoStackImages::continuousShooting(DB::FileNameList &toBeShown )
{
    DB::ImageInfoPtr prev;
    Q_FOREACH(const DB::FileName& fileName, m_list) {
        DB::ImageInfoPtr info = fileName.info();
        // Skipping images that do not have exact time stamp
        if ( info->date().start() != info->date().end() )
            continue;
        if ( prev && ( prev->date().start().secsTo( info->date().start() ) < m_continuousThreshold->value() ) ) {
            DB::FileNameList stack;

            if ( !DB::ImageDB::instance()->getStackFor( prev->fileName() ).isEmpty() ) {
                if ( m_autostackUnstack->isChecked() )
                    DB::ImageDB::instance()->unstack( DB::FileNameList() << prev->fileName());
                else if ( m_autostackSkip->isChecked() )
                    continue;
            }

            if ( !DB::ImageDB::instance()->getStackFor(fileName).isEmpty() ) {
                if ( m_autostackUnstack->isChecked() )
                    DB::ImageDB::instance()->unstack( DB::FileNameList() << fileName);
                else if ( m_autostackSkip->isChecked() )
                    continue;
            }

            stack.append(prev->fileName());
            stack.append(info->fileName());
            if ( !toBeShown.isEmpty() ) {
                if ( toBeShown.at( toBeShown.size() - 1 ).info()->fileName() != prev->fileName() )
                    toBeShown.append(prev->fileName());
            } else {
                // if this is first insert, we have to include also the stacked images from previuous image
                if ( !DB::ImageDB::instance()->getStackFor( info->fileName() ).isEmpty() )
                    Q_FOREACH( const DB::FileName& a, DB::ImageDB::instance()->getStackFor( prev->fileName() ) )
                        toBeShown.append( a );
                else
                    toBeShown.append(prev->fileName());
            }
            // Inserting stacked images from the current image
            if ( !DB::ImageDB::instance()->getStackFor( info->fileName() ).isEmpty() )
                Q_FOREACH( const DB::FileName& a, DB::ImageDB::instance()->getStackFor(fileName))
                    toBeShown.append( a );
            else
                toBeShown.append(info->fileName());
            DB::ImageDB::instance()->stack(stack);
        }

        prev = info;
    }
}

void AutoStackImages::accept()
{
    QDialog::accept();
    Utilities::ShowBusyCursor dummy;
    DB::FileNameList toBeShown;

    if ( m_matchingMD5->isChecked() )
        matchingMD5( toBeShown );
    if ( m_matchingFile->isChecked() )
        matchingFile( toBeShown );
    if ( m_continuousShooting->isChecked() )
        continuousShooting( toBeShown );

    MainWindow::Window::theMainWindow()->showThumbNails(toBeShown);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
