#include "VisibleOptionsMenu.h"
#include "Settings/SettingsData.h"
#include <KToggleAction>
#include <KActionCollection>
#include <klocale.h>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QList>
#include <QPalette>
#include "DB/Category.h"
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"

Viewer::VisibleOptionsMenu::VisibleOptionsMenu(QWidget* parent, KActionCollection* actions)
    : QMenu(i18n("Show..."), parent)
{
    setTearOffEnabled(true);
    setTitle( i18n("Show") );
    connect( this, SIGNAL( aboutToShow() ), this, SLOT( updateState() ) );

    _showInfoBox = actions->add<KToggleAction>( QString::fromLatin1("viewer-show-infobox") );
    _showInfoBox->setText( i18n("Show Info Box") );
    _showInfoBox->setShortcut( Qt::CTRL+Qt::Key_I );
    connect( _showInfoBox, SIGNAL( toggled(bool) ), this, SLOT( toggleShowInfoBox( bool ) ) );
    addAction( _showInfoBox );

    _showLabel = actions->add<KToggleAction>( QString::fromLatin1("viewer-show-label") );
    _showLabel->setText( i18n("Show Label") );
    _showLabel->setShortcut( 0 );
    connect( _showLabel, SIGNAL( toggled(bool) ), this, SLOT( toggleShowLabel( bool ) ) );
    addAction( _showLabel );

    _showDescription = actions->add<KToggleAction>( QString::fromLatin1("viewer-show-description") );
    _showDescription->setText( i18n("Show Description") );
    _showDescription->setShortcut( 0 );
    connect( _showDescription, SIGNAL( toggled(bool) ), this, SLOT( toggleShowDescription( bool ) ) );
    addAction( _showDescription );

    _showDate = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-date") );
    _showDate->setText( i18n("Show Date") );
    connect( _showDate, SIGNAL( toggled(bool) ), this, SLOT( toggleShowDate( bool ) ) );
    addAction( _showDate );

    _showTime = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-time") );
    _showTime->setText( i18n("Show Time") );
    connect( _showTime, SIGNAL( toggled(bool) ), this, SLOT( toggleShowTime( bool ) ) );
    addAction( _showTime );

    _showFileName = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-filename") );
    _showFileName->setText( i18n("Show Filename") );
    connect( _showFileName, SIGNAL( toggled(bool) ), this, SLOT( toggleShowFilename( bool ) ) );
    addAction( _showFileName );

    _showExif = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-exif") );
    _showExif->setText( i18n("Show EXIF") );
    connect( _showExif, SIGNAL( toggled(bool) ), this, SLOT( toggleShowEXIF( bool ) ) );
    addAction( _showExif );

    _showImageSize = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-imagesize") );
    _showImageSize->setText( i18n("Show Image Size") );
    connect( _showImageSize, SIGNAL( toggled(bool) ), this, SLOT( toggleShowImageSize( bool ) ) );
    addAction( _showImageSize );

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        KToggleAction* taction = actions->add<KToggleAction>( (*it)->name() );
        _actionList.append( taction );
        taction->setText( (*it)->text() );
        taction->setData( (*it)->name() );
        addAction( taction );
        connect( taction, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowCategory( bool ) ) );
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

void Viewer::VisibleOptionsMenu::toggleShowInfoBox( bool b )
{
    Settings::SettingsData::instance()->setShowInfoBox( b );
    emit visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::updateState()
{
    _showInfoBox->setChecked( Settings::SettingsData::instance()->showInfoBox() );
    _showLabel->setChecked( Settings::SettingsData::instance()->showLabel() );
    _showDescription->setChecked( Settings::SettingsData::instance()->showDescription() );
    _showDate->setChecked( Settings::SettingsData::instance()->showDate() );
    _showTime->setChecked( Settings::SettingsData::instance()->showTime() );
    _showFileName->setChecked( Settings::SettingsData::instance()->showFilename() );
    _showExif->setChecked( Settings::SettingsData::instance()->showEXIF() );
    _showImageSize->setChecked( Settings::SettingsData::instance()->showImageSize() );

    Q_FOREACH( KToggleAction* action, _actionList ) {
        action->setChecked( DB::ImageDB::instance()->categoryCollection()->categoryForName(action->data().value<QString>())->doShow() );
    }
}


