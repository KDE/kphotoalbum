/* Copyright (C) 2003-2018 Jesper K Pedersen <blackie@kde.org>

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

#include "WelcomeDialog.h"
#include "FeatureDialog.h"
#include "Window.h"
#include <Utilities/Util.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KShell>

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

using namespace MainWindow;

WelcomeDialog::WelcomeDialog( QWidget* parent )
    : QDialog( parent )

{
    QVBoxLayout* lay1 = new QVBoxLayout( this );
    QHBoxLayout* lay2 = new QHBoxLayout;
    lay1->addLayout( lay2 );

    QLabel* image = new QLabel( this );
    image->setMinimumSize( QSize( 273, 204 ) );
    image->setMaximumSize( QSize( 273, 204 ) );
    image->setPixmap(Utilities::locateDataFile(QString::fromLatin1("pics/splash.png")));
    lay2->addWidget( image );

    QLabel* textLabel2 = new QLabel( this );
    lay2->addWidget( textLabel2 );
    textLabel2->setText( i18n( "<h1>Welcome to KPhotoAlbum</h1>"
                               "<p>KPhotoAlbum is a powerful free tool to archive, tag and manage your photos and "
                               "videos. It will not modify or change your precious files, it only indexes them "
                               "and lets you easily find and manage your photos and videos.</p>"
                               "<p>Start by showing KPhotoAlbum where your photos are by pressing on Create My Own "
                               "Database. Select this button also if you have an existing KPhotoAlbum database "
                               "that you want to start using again.</p>"
                               "<p>If you feel safer first trying out KPhotoAlbum with prebuilt set of images, "
                               "press the Load Demo button.</p>" ) );
    textLabel2->setWordWrap( true );

    QHBoxLayout* lay3 = new QHBoxLayout;
    lay1->addLayout( lay3 );
    lay3->addStretch( 1 );

    QPushButton* createSetup = new QPushButton( i18n("Create My Own Database..."), this );
    lay3->addWidget( createSetup );

    QPushButton* loadDemo = new QPushButton( i18n("Load Demo") );
    lay3->addWidget( loadDemo );

    QPushButton* checkFeatures = new QPushButton( i18n("Check My Feature Set") );
    lay3->addWidget( checkFeatures );

    connect(loadDemo, &QPushButton::clicked, this, &WelcomeDialog::slotLoadDemo);
    connect(createSetup, &QPushButton::clicked, this, &WelcomeDialog::createSetup);
    connect(checkFeatures, &QPushButton::clicked, this, &WelcomeDialog::checkFeatures);
}


void WelcomeDialog::slotLoadDemo()
{
    // rerun KPA with "--demo"
    MainWindow::Window::theMainWindow()->runDemo();
    // cancel the dialog (and exit this instance of KPA)
    reject();
}

void WelcomeDialog::createSetup()
{
    FileDialog dialog( this );
    m_configFile = dialog.getFileName();
    if ( !m_configFile.isNull() )
        accept();
}

QString WelcomeDialog::configFileName() const
{
    return m_configFile;
}

FileDialog::FileDialog( QWidget* parent ) :QDialog( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout (this);

    QLabel* label = new QLabel( i18n("<h1>KPhotoAlbum database creation</h1>"
                                     "<p>You need to show where the photos and videos are for KPhotoAlbum to "
                                     "find them. They all need to be under one root directory, for example "
                                     "/home/user/Images. In this directory you can have as many subdirectories as you "
                                     "want, KPhotoAlbum will find them all for you.</p>"
                                     "<p>Feel safe, KPhotoAlbum will not modify or edit any of your images, so you can "
                                     "simply point KPhotoAlbum to the directory where you already have all your "
                                     "images.</p>"
                                     "<p>If you have an existing KPhotoAlbum database and root directory somewhere, "
                                     "point KPhotoAlbum to that directory to start using it again.</p>" ), this );
    label->setWordWrap( true );
    mainLayout->addWidget( label );

    QHBoxLayout* lay2 = new QHBoxLayout;
    label = new QLabel( i18n("Image/Video root directory: "), this );
    lay2->addWidget( label );

    m_lineEdit = new QLineEdit( this );
    m_lineEdit->setText( QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) );
    lay2->addWidget( m_lineEdit );

    QPushButton* button = new QPushButton( QString::fromLatin1("..."), this );
    button->setMaximumWidth( 30 );
    lay2->addWidget( button );
    connect(button, &QPushButton::clicked, this, &FileDialog::slotBrowseForDirecory);

    mainLayout->addLayout( lay2 );


    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FileDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FileDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void FileDialog::slotBrowseForDirecory()
{
    QString dir = QFileDialog::getExistingDirectory(this , QString(), m_lineEdit->text());
    if ( ! dir.isNull() )
        m_lineEdit->setText( dir );
}

QString FileDialog::getFileName()
{
    bool ok = false;
    QString dir;
    while ( !ok ) {
        if ( exec() == Rejected )
            return QString();

        dir =  KShell::tildeExpand( m_lineEdit->text() );
        if ( !QFileInfo( dir ).exists() ) {
            int create = KMessageBox::questionYesNo( this, i18n("Directory does not exist, create it?") );
            if ( create == KMessageBox::Yes ) {
                bool ok2 = QDir().mkdir( dir );
                if ( !ok2 ) {
                    KMessageBox::sorry( this, i18n("Could not create directory %1",dir) );
                }
                else
                    ok = true;
            }
        }
        else if ( !QFileInfo( dir ).isDir() ) {
            KMessageBox::sorry( this, i18n("%1 exists, but is not a directory",dir) );
        }
        else
            ok = true;
    }

    QString file = dir + QString::fromLatin1("/index.xml");
    KConfigGroup group = KSharedConfig::openConfig()->group(QString::fromUtf8("General"));
    group.writeEntry( QString::fromLatin1("imageDBFile"), file );
    group.sync();


    return file;
}

void MainWindow::WelcomeDialog::checkFeatures()
{
    if ( !FeatureDialog::hasAllFeaturesAvailable() ) {
        const QString msg =
            i18n("<p>KPhotoAlbum does not seem to be build with support for all its features. The following is a list "
                 "indicating to you what you may miss:<ul>%1</ul></p>"
                 "<p>For details on how to solve this problem, please choose <b>Help</b>|<b>KPhotoAlbum Feature Status</b> "
                 "from the menus.</p>", FeatureDialog::featureString() );
        KMessageBox::information( this, msg, i18n("Feature Check") );
    }
    else {
        KMessageBox::information( this, i18n("Congratulations: all dynamic features have been enabled."),
                                  i18n("Feature Check" ) );
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
