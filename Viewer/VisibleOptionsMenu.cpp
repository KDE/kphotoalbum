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
#include "VisibleOptionsMenu.h"
#include "Settings/SettingsData.h"
#include <KToggleAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <QCheckBox>
#include <QList>
#include "DB/Category.h"
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"

Viewer::VisibleOptionsMenu::VisibleOptionsMenu(QWidget* parent, KActionCollection* actions)
    : QMenu(i18n("Show..."), parent)
{
    setTearOffEnabled(true);
    setTitle( i18n("Show") );
    connect(this, &VisibleOptionsMenu::aboutToShow, this, &VisibleOptionsMenu::updateState);

    m_showInfoBox = actions->add<KToggleAction>( QString::fromLatin1("viewer-show-infobox") );
    m_showInfoBox->setText( i18n("Show Info Box") );
    m_showInfoBox->setShortcut( Qt::CTRL+Qt::Key_I );
    m_showInfoBox->setChecked(Settings::SettingsData::instance()->showInfoBox());
    connect(m_showInfoBox, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowInfoBox);
    addAction( m_showInfoBox );

    m_showLabel = actions->add<KToggleAction>( QString::fromLatin1("viewer-show-label") );
    m_showLabel->setText( i18n("Show Label") );
    m_showLabel->setShortcut( 0 );
    connect(m_showLabel, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowLabel);
    addAction( m_showLabel );

    m_showDescription = actions->add<KToggleAction>( QString::fromLatin1("viewer-show-description") );
    m_showDescription->setText( i18n("Show Description") );
    m_showDescription->setShortcut( 0 );
    connect(m_showDescription, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowDescription);
    addAction( m_showDescription );

    m_showDate = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-date") );
    m_showDate->setText( i18n("Show Date") );
    connect(m_showDate, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowDate);
    addAction( m_showDate );

    m_showTime = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-time") );
    m_showTime->setText( i18n("Show Time") );
    connect(m_showTime, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowTime);
    addAction( m_showTime );
    m_showTime->setVisible( m_showDate->isChecked() );

    m_showFileName = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-filename") );
    m_showFileName->setText( i18n("Show Filename") );
    connect(m_showFileName, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowFilename);
    addAction( m_showFileName );

    m_showExif = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-exif") );
    m_showExif->setText( i18n("Show Exif") );
    connect(m_showExif, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowEXIF);
    addAction( m_showExif );

    m_showImageSize = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-imagesize") );
    m_showImageSize->setText( i18n("Show Image Size") );
    connect(m_showImageSize, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowImageSize);
    addAction( m_showImageSize );

    m_showRating = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-rating") );
    m_showRating->setText( i18n("Show Rating") );
    connect(m_showRating, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowRating);
    addAction( m_showRating );

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        KToggleAction* taction = actions->add<KToggleAction>( (*it)->name() );
        m_actionList.append( taction );
        taction->setText( (*it)->name() );
        taction->setData( (*it)->name() );
        addAction( taction );
        connect(taction, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowCategory);
    }
}

void Viewer::VisibleOptionsMenu::toggleShowCategory( bool b )
{
    QAction* action = qobject_cast<QAction*>(sender() );
    DB::ImageDB::instance()->categoryCollection()->categoryForName(action->data().value<QString>())->setDoShow( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowLabel( bool b )
{
    Settings::SettingsData::instance()->setShowLabel( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowDescription( bool b )
{
    Settings::SettingsData::instance()->setShowDescription( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowDate( bool b )
{
    Settings::SettingsData::instance()->setShowDate( b );
    m_showTime->setVisible( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowFilename( bool b )
{
    Settings::SettingsData::instance()->setShowFilename( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowTime( bool b )
{
    Settings::SettingsData::instance()->setShowTime( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowEXIF( bool b )
{
    Settings::SettingsData::instance()->setShowEXIF( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowImageSize( bool b )
{
    Settings::SettingsData::instance()->setShowImageSize( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowRating( bool b )
{
    Settings::SettingsData::instance()->setShowRating( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowInfoBox( bool b )
{
    Settings::SettingsData::instance()->setShowInfoBox( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::updateState()
{
    m_showInfoBox->setChecked( Settings::SettingsData::instance()->showInfoBox() );
    m_showLabel->setChecked( Settings::SettingsData::instance()->showLabel() );
    m_showDescription->setChecked( Settings::SettingsData::instance()->showDescription() );
    m_showDate->setChecked( Settings::SettingsData::instance()->showDate() );
    m_showTime->setChecked( Settings::SettingsData::instance()->showTime() );
    m_showFileName->setChecked( Settings::SettingsData::instance()->showFilename() );
    m_showExif->setChecked( Settings::SettingsData::instance()->showEXIF() );
    m_showImageSize->setChecked( Settings::SettingsData::instance()->showImageSize() );
    m_showRating->setChecked( Settings::SettingsData::instance()->showRating() );

    Q_FOREACH( KToggleAction* action, m_actionList ) {
        action->setChecked( DB::ImageDB::instance()->categoryCollection()->categoryForName(action->data().value<QString>())->doShow() );
    }
}


// vi:expandtab:tabstop=4 shiftwidth=4:
