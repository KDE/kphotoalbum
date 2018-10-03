/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "InvalidDateFinder.h"
#include <qlayout.h>
#include <qradiobutton.h>
#include <KLocalizedString>
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/ImageDate.h"
#include "DB/FileInfo.h"
#include "MainWindow/Window.h"
#include <qapplication.h>
#include <qeventloop.h>
#include "Utilities/ShowBusyCursor.h"
#include <QGroupBox>
#include <KTextEdit>
#include <QProgressDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace MainWindow;

InvalidDateFinder::InvalidDateFinder( QWidget* parent )
    :QDialog( parent )
{
    setWindowTitle( i18nc("@title:window", "Search for Images and Videos with Missing Dates" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QGroupBox* grp = new QGroupBox( i18n("Which Images and Videos to Display") );
    QVBoxLayout* grpLay = new QVBoxLayout( grp );
    mainLayout->addWidget( grp );

    m_dateNotTime = new QRadioButton( i18n( "Search for images and videos with a valid date but an invalid time stamp") );
    m_missingDate = new QRadioButton( i18n( "Search for images and videos missing date and time" ) );
    m_partialDate = new QRadioButton( i18n( "Search for images and videos with only partial dates (like 1971 vs. 11/7-1971)") );
    m_dateNotTime->setChecked( true );

    grpLay->addWidget( m_dateNotTime );
    grpLay->addWidget( m_missingDate );
    grpLay->addWidget( m_partialDate );

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &InvalidDateFinder::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &InvalidDateFinder::reject);
    mainLayout->addWidget(buttonBox);
}

void InvalidDateFinder::accept()
{
    QDialog::accept();
    Utilities::ShowBusyCursor dummy;

    // create the info dialog
    QDialog* info = new QDialog;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    info->setLayout(mainLayout);
    info->setWindowTitle( i18nc("@title:window", "Image Info" ) );

    KTextEdit* edit = new KTextEdit( info );
    mainLayout->addWidget( edit );
    edit->setText( i18n("<h1>Here you may see the date changes for the displayed items.</h1>") );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    info->connect(buttonBox, &QDialogButtonBox::accepted, info, &QDialog::accept);
    info->connect(buttonBox, &QDialogButtonBox::rejected, info, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Now search for the images.
    const DB::FileNameList list = DB::ImageDB::instance()->images();
    DB::FileNameList toBeShown;
    QProgressDialog dialog( nullptr);
    dialog.setWindowTitle(i18nc("@title:window", "Reading File Properties"));
    dialog.setMaximum(list.size());
    dialog.setValue(0);
    int progress = 0;

    Q_FOREACH(const DB::FileName& fileName, list) {
        dialog.setValue( ++progress );
        qApp->processEvents( QEventLoop::AllEvents );
        if ( dialog.wasCanceled() )
            break;
        if ( fileName.info()->isNull() )
            continue;

        DB::ImageDate date = fileName.info()->date();
        bool show = false;
        if ( m_dateNotTime->isChecked() ) {
            DB::FileInfo fi = DB::FileInfo::read( fileName, DB::EXIFMODE_DATE );
            if ( fi.dateTime().date() == date.start().date() )
                show = ( fi.dateTime().time() != date.start().time() );
            if ( show ) {
                edit->append( QString::fromLatin1("%1:<br/>existing = %2<br>new..... = %3" )
                              .arg(fileName.relative())
                              .arg(date.start().toString())
                              .arg(fi.dateTime().toString()) );
            }
        }
        else if ( m_missingDate->isChecked() ) {
            show = !date.start().isValid();
        }
        else if ( m_partialDate->isChecked() ) {
            show = ( date.start() != date.end() );
        }

        if ( show )
            toBeShown.append(fileName);
    }

    if ( m_dateNotTime->isChecked() ) {
        info->resize( 800, 600 );
        edit->setReadOnly( true );
        QFont f = edit->font();
        f.setFamily( QString::fromLatin1( "fixed" ) );
        edit->setFont( f );
        info->show();
    }
    else
        delete info;

    Window::theMainWindow()->showThumbNails( toBeShown );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
