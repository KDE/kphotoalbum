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

#include "Dialog.h"
#include <QStackedWidget>
#include <KAction>
#include <KActionCollection>
#include <KComboBox>
#include <QList>
#include <QCloseEvent>
#include <QDir>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <QSpinBox>
#include <kacceleratormanager.h>
#include <kguiitem.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktextedit.h>
#include <QMenu>
#include <qapplication.h>
#include <qcursor.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpoint.h>

#include <kratingwidget.h>

#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "ImagePreviewWidget.h"
#include "KDateEdit.h"
#include "ListSelect.h"
#include "MainWindow/DirtyIndicator.h"
#include "Settings/SettingsData.h"
#include "ShortCutManager.h"
#include "ShowSelectionOnlyManager.h"
#include "Utilities/ShowBusyCursor.h"
#include "Utilities/Util.h"
#include "Viewer/ViewerWidget.h"
#include "enums.h"
#include "ResizableFrame.h"

#include "DescriptionEdit.h"
#include <QDebug>

using Utilities::StringSet;

/**
 * \class AnnotationDialog::Dialog
 * \brief QDialog subclass used for tagging images
 */

AnnotationDialog::Dialog::Dialog( QWidget* parent )
    : KDialog( parent ),
    _ratingChanged( false ),
    conflictText( i18n("(You have differing descriptions on individual images, setting text here will override them all)" ) )
{
    Utilities::ShowBusyCursor dummy;
    ShortCutManager shortCutManager;

    // The widget stack
    _stack = new QStackedWidget( mainWidget() );
    QVBoxLayout* layout = new QVBoxLayout( mainWidget() );
    layout->addWidget( _stack );

    // The Viewer
    _fullScreenPreview = new Viewer::ViewerWidget( Viewer::ViewerWidget::InlineViewer );
    _stack->addWidget( _fullScreenPreview );

    // The dock widget
    _dockWindow = new QMainWindow;
    _stack->addWidget( _dockWindow );
    _dockWindow->setDockNestingEnabled( true );

    // -------------------------------------------------- Dock widgets
    createDock( i18n("Label and Dates"), QString::fromLatin1("Label and Dates"), Qt::TopDockWidgetArea, createDateWidget(shortCutManager) );

    createDock( i18n("Image Preview"), QString::fromLatin1("Image Preview"), Qt::TopDockWidgetArea, createPreviewWidget() );

    _description = new DescriptionEdit(this);
    _description->setProperty( "WantsFocus", true );
    _description->setObjectName( i18n("Description") );
    _description->setCheckSpellingEnabled( true );
    _description->setTabChangesFocus( true ); // this allows tabbing to the next item in the tab order.

    QDockWidget* dock = createDock( i18n("Description"), QString::fromLatin1("description"), Qt::LeftDockWidgetArea, _description );
    shortCutManager.addDock( dock, _description );

    connect( _description, SIGNAL(pageUpDownPressed(QKeyEvent*)), this, SLOT(descriptionPageUpDownPressed(QKeyEvent*)) );

    // -------------------------------------------------- Categories
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();

    // Let's first assume we don't have positionable categories
    _positionableCategories = false;

    for( QList<DB::CategoryPtr>::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt ) {
        ListSelect* sel = createListSel( *categoryIt );

        // Create a QMap of all ListSelect instances, so that we can easily
        // check if a specific (positioned) tag is (still) selected later
        _listSelectList[(*categoryIt)->name()] = sel;

        QDockWidget* dock = createDock( (*categoryIt)->text(), (*categoryIt)->name(), Qt::BottomDockWidgetArea, sel );
        shortCutManager.addDock( dock, sel->lineEdit() );

        if ( (*categoryIt)->isSpecialCategory() )
            dock->hide();

        // Pass the positionable selection to the object
        sel->setPositionable( (*categoryIt)->positionable() );

        if ( sel->positionable() ) {
            connect( sel, SIGNAL(positionableTagSelected(QString,QString)), this, SLOT(positionableTagSelected(QString,QString)) );
            connect( sel, SIGNAL(positionableTagDeselected(QString,QString)), this, SLOT(positionableTagDeselected(QString,QString)) );
            connect( sel, SIGNAL(positionableTagRenamed(QString,QString,QString)), this, SLOT(positionableTagRenamed(QString,QString,QString)) );

            // We have at least one positionable category
            _positionableCategories = true;
        }

        // The category could have a localized name. Perhaps, this could be
        // also handy for something else, so let's do this for all categories
        _categoryL10n[(*categoryIt)->name()] = (*categoryIt)->text();
    }

    // -------------------------------------------------- The buttons.
    // don't use default buttons (Ok, Cancel):
    setButtons( None );
    QHBoxLayout* lay1 = new QHBoxLayout;
    layout->addLayout( lay1 );

    _revertBut = new KPushButton( i18n("Revert This Item") );
    KAcceleratorManager::setNoAccel(_revertBut);
    lay1->addWidget( _revertBut );

    _clearBut = new KPushButton( KGuiItem(i18n("Clear Form"),QApplication::isRightToLeft()
                                             ? QString::fromLatin1("clear_left")
                                             : QString::fromLatin1("locationbar_erase")) );
    KAcceleratorManager::setNoAccel(_clearBut);
    lay1->addWidget( _clearBut );

    KPushButton* optionsBut = new KPushButton( i18n("Options..." ) );
    KAcceleratorManager::setNoAccel(optionsBut);
    lay1->addWidget( optionsBut );

    lay1->addStretch(1);

    _okBut = new KPushButton( i18n("&Done") );
    lay1->addWidget( _okBut );

    _continueLaterBut = new KPushButton( i18n("Continue &Later") );
    lay1->addWidget( _continueLaterBut );

    KPushButton* cancelBut = new KPushButton( KStandardGuiItem::cancel() );
    lay1->addWidget( cancelBut );

    // It is unfortunately not possible to ask KAcceleratorManager not to setup the OK and cancel keys.
    shortCutManager.addTaken( i18nc("@action:button","&Search") );
    shortCutManager.addTaken( _okBut->text() );
    shortCutManager.addTaken( _continueLaterBut->text());
    shortCutManager.addTaken( cancelBut->text() );

    connect( _revertBut, SIGNAL(clicked()), this, SLOT(slotRevert()) );
    connect( _okBut, SIGNAL(clicked()), this, SLOT(doneTagging()) );
    connect( _continueLaterBut, SIGNAL(clicked()), this, SLOT(continueLater()) );
    connect( cancelBut, SIGNAL(clicked()), this, SLOT(reject()) );
    connect( _clearBut, SIGNAL(clicked()), this, SLOT(slotClear()) );
    connect( optionsBut, SIGNAL(clicked()), this, SLOT(slotOptions()) );

    connect( _preview, SIGNAL(imageRotated(int)), this, SLOT(rotate(int)) );
    connect( _preview, SIGNAL(indexChanged(int)), this, SLOT(slotIndexChanged(int)) );
    connect( _preview, SIGNAL(imageDeleted(DB::ImageInfo)), this, SLOT(slotDeleteImage()) );
    connect( _preview, SIGNAL(copyPrevClicked()), this, SLOT(slotCopyPrevious()) );
    connect( _preview, SIGNAL(areaVisibilityChanged(bool)), this, SLOT(slotShowAreas(bool)) );
    connect( _preview->preview(), SIGNAL(areaCreated(ResizableFrame*)), this, SLOT(slotNewArea(ResizableFrame*)) );

    // Disable so no button accept return (which would break with the line edits)
    _revertBut->setAutoDefault( false );
    _okBut->setAutoDefault( false );
    _continueLaterBut->setAutoDefault( false );
    cancelBut->setAutoDefault( false );
    _clearBut->setAutoDefault( false );
    optionsBut->setAutoDefault( false );

    _dockWindowCleanState = _dockWindow->saveState();

    loadWindowLayout();

    _current = -1;

    setGeometry( Settings::SettingsData::instance()->windowGeometry( Settings::AnnotationDialog ) );

    setupActions();
    shortCutManager.setupShortCuts();
}

QDockWidget* AnnotationDialog::Dialog::createDock( const QString& title, const QString& name,
                                                   Qt::DockWidgetArea location, QWidget* widget )
{
    QDockWidget* dock = new QDockWidget( title );
    KAcceleratorManager::setNoAccel(dock);
    dock->setObjectName( name );
    dock->setAllowedAreas( Qt::AllDockWidgetAreas );
    dock->setWidget( widget );
    _dockWindow->addDockWidget( location, dock );
    _dockWidgets.append( dock );
    return dock;
}

QWidget* AnnotationDialog::Dialog::createDateWidget(ShortCutManager& shortCutManager)
{
    QWidget* top = new QWidget;
    QVBoxLayout* lay2 = new QVBoxLayout( top );

    // Image Label
    QHBoxLayout* lay3 = new QHBoxLayout;
    lay2->addLayout( lay3 );

    QLabel* label = new QLabel( i18n("Label: " ) );
    lay3->addWidget( label );
    _imageLabel = new KLineEdit;
    _imageLabel->setProperty( "WantsFocus", true );
    _imageLabel->setObjectName( i18n("Label") );
    lay3->addWidget( _imageLabel );
    shortCutManager.addLabel( label );
    label->setBuddy( _imageLabel );


    // Date
    QHBoxLayout* lay4 = new QHBoxLayout;
    lay2->addLayout( lay4 );

    label = new QLabel( i18n("Date: ") );
    lay4->addWidget( label );

    _startDate = new ::AnnotationDialog::KDateEdit( true );
    lay4->addWidget( _startDate, 1 );
    connect( _startDate, SIGNAL(dateChanged(DB::ImageDate)), this, SLOT(slotStartDateChanged(DB::ImageDate)) );
    shortCutManager.addLabel(label );
    label->setBuddy( _startDate);

    _endDateLabel = new QLabel( QString::fromLatin1( "-" ) );
    lay4->addWidget( _endDateLabel );

    _endDate = new ::AnnotationDialog::KDateEdit( false );
    lay4->addWidget( _endDate, 1 );

    // Time
    _timeLabel = new QLabel( i18n("Time: ") );
    lay4->addWidget( _timeLabel );

    _time= new QTimeEdit;
    lay4->addWidget( _time );

    _isFuzzyDate = new QCheckBox( i18n("Use Fuzzy Date") );
    _isFuzzyDate->setWhatsThis( i18nc("@info",
                "<para>In KPhotoAlbum, images can either have an exact date and time"
                ", or a <emphasis>fuzzy</emphasis> date which happened any time during"
                " a specified time interval. Images produced by digital cameras"
                " do normally have an exact date.</para>"
                "<para>If you don't know exactly when a photo was taken"
                " (e.g. if the photo comes from an analog camera), then you should set"
                " <interface>Use Fuzzy Date</interface>.</para>") );
    _isFuzzyDate->setToolTip( _isFuzzyDate->whatsThis() );
    lay4->addWidget( _isFuzzyDate );
    lay4->addStretch(1);
    connect(_isFuzzyDate,SIGNAL(stateChanged(int)),this,SLOT(slotSetFuzzyDate()));

    QHBoxLayout* lay8 = new QHBoxLayout;
    lay2->addLayout( lay8 );

    _megapixelLabel = new QLabel( i18n("Minimum megapixels:") );
    lay8->addWidget( _megapixelLabel );

    _megapixel = new QSpinBox;
    _megapixel->setRange( 0, 99 );
    _megapixel->setSingleStep( 1 );
    _megapixelLabel->setBuddy( _megapixel );
    lay8->addWidget( _megapixel );
    lay8->addStretch( 1 );

    QHBoxLayout* lay9 = new QHBoxLayout;
    lay2->addLayout( lay9 );

    label = new QLabel( i18n("Rating:") );
    lay9->addWidget( label );
    _rating = new KRatingWidget;
    _rating->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    lay9->addWidget( _rating, 0, Qt::AlignCenter );
    connect( _rating, SIGNAL(ratingChanged(uint)), this, SLOT(slotRatingChanged(uint)) );

    _ratingSearchLabel = new QLabel( i18n("Rating search mode:") );
    lay9->addWidget( _ratingSearchLabel );

    _ratingSearchMode = new KComboBox( lay9 );
    _ratingSearchMode->addItems( QStringList() << i18n("==") << i18n("&gt;=") << i18n("&lt;=") << i18n("!=") );
    _ratingSearchLabel->setBuddy( _ratingSearchMode );
    lay9->addWidget( _ratingSearchMode );

    // File name search pattern
    QHBoxLayout* lay10 = new QHBoxLayout;
    lay2->addLayout( lay10 );

    _imageFilePatternLabel = new QLabel( i18n("File Name Pattern: " ) );
    lay10->addWidget( _imageFilePatternLabel );
    _imageFilePattern = new KLineEdit;
    _imageFilePattern->setObjectName( i18n("File Name Pattern") );
    lay10->addWidget( _imageFilePattern );
    shortCutManager.addLabel( _imageFilePatternLabel );
    _imageFilePatternLabel->setBuddy( _imageFilePattern );

    _searchRAW = new QCheckBox( i18n("Search only for RAW files") );
    lay2->addWidget( _searchRAW );

    lay9->addStretch( 1 );
    lay2->addStretch(1);

    return top;
}

QWidget* AnnotationDialog::Dialog::createPreviewWidget()
{
    _preview = new ImagePreviewWidget();
    return _preview;
}

void AnnotationDialog::Dialog::slotRevert()
{
    if ( _setup == InputSingleImageConfigMode )
        load();
}

void AnnotationDialog::Dialog::slotIndexChanged( int index )
{
  if ( _setup != InputSingleImageConfigMode )
        return;

    if(_current >= 0 )
      writeToInfo();

    _current = index;

    load();
}

void AnnotationDialog::Dialog::doneTagging()
{
    saveAndClose();
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        for( DB::ImageInfoListIterator it = _origList.begin(); it != _origList.end(); ++it ) {
            (*it)->removeCategoryInfo( Settings::SettingsData::instance()->untaggedCategory(),
                                      Settings::SettingsData::instance()->untaggedTag() );
        }
    }
}

/*
 * Copy tags (only tags/categories, not description/label/...) from previous image to the currently showed one
 */
void AnnotationDialog::Dialog::slotCopyPrevious()
{
    if ( _setup != InputSingleImageConfigMode )
        return;
    if ( _current < 1 )
        return;

    // FIXME: it would be better to compute the "previous image" in a better way, but let's stick with this for now...
    DB::ImageInfo& old_info = _editList[ _current - 1 ];

    _positionableTagCandidates.clear();
    _lastSelectedPositionableTag.first = QString();
    _lastSelectedPositionableTag.second = QString();
    QList<ResizableFrame *> allAreas = _preview->preview()->findChildren<ResizableFrame *>();

    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
        (*it)->setSelection( old_info.itemsOfCategory( (*it)->category() ) );

        // Also set all positionable tag candidates

        if ( (*it)->positionable() ) {
            QString category = (*it)->category();
            QSet<QString> selectedTags = old_info.itemsOfCategory( category );

            for ( const auto tag : selectedTags ) {
                QRect area = _editList[_current].areaForTag(category, tag);
                if (area.isNull()) {
                    // no associated area yet
                    addTagToCandidateList(category, tag);
                }
            }

            // Check all areas for a linked tag in this category that is probably not selected anymore
            for(ResizableFrame *area : allAreas) {
                QPair<QString, QString> tagData = area->tagData();

                if (tagData.first == category) {
                    if (! selectedTags.contains(tagData.second)) {
                        // The linked tag is not selected anymore, so remove it
                        area->removeTagData();
                    }
                }
            }
        }
    }
}

void AnnotationDialog::Dialog::load()
{
    // Remove all areas
    tidyAreas();
    // Empty the positionable tag candidate list
    _positionableTagCandidates.clear();

    DB::ImageInfo& info = _editList[ _current ];
    _startDate->setDate( info.date().start().date() );

    if( info.date().hasValidTime() ) {
        _time->show();
        _time->setTime( info.date().start().time());
        _isFuzzyDate->setChecked(false);
    }
    else {
        _time->hide();
        _isFuzzyDate->setChecked(true);
    }

    if ( info.date().start().date() == info.date().end().date() )
        _endDate->setDate( QDate() );
    else
        _endDate->setDate( info.date().end().date() );

    _imageLabel->setText( info.label() );
    _description->setPlainText( info.description() );

    if ( _setup == InputSingleImageConfigMode )
        _rating->setRating( qMax( static_cast<short int>(0), info.rating() ) );
    _ratingChanged = false;

    // A category areas have been linked against could have been deleted
    // or un-marked as positionable in the meantime, so ...
    QMap<QString, bool> categoryIsPositionable;

    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
        (*it)->setSelection( info.itemsOfCategory( (*it)->category() ) );
        (*it)->rePopulate();

        // Get all selected positionable tags and add them to the candidate list
        if ((*it)->positionable()) {
            QSet<QString> selectedTags = (*it)->itemsOn();
            QSet<QString>::iterator tagName;

            for (tagName = selectedTags.begin(); tagName != selectedTags.end(); ++tagName) {
                addTagToCandidateList( (*it)->category(), *tagName );
            }
        }

        // ... create a list of all categories and their positionability ...
        categoryIsPositionable[(*it)->category()] = (*it)->positionable();
    }

    // Create all tagged areas

    QMap<QString, QMap<QString, QRect>> taggedAreas = info.taggedAreas();
    QMapIterator<QString, QMap<QString, QRect>> areasInCategory(taggedAreas);

    while (areasInCategory.hasNext()) {
        areasInCategory.next();
        QString category = areasInCategory.key();

        // ... and check if the respective category is actually there yet and still positionable
        // (operator[] will insert an empty item if the category has been deleted
        // and is thus missing in the QMap, but the respective key won't be true)
        if (categoryIsPositionable[category]) {
            QMapIterator<QString, QRect> areaData(areasInCategory.value());
            while (areaData.hasNext()) {
                areaData.next();
                QString tag = areaData.key();

                // Be sure that the corresponding tag is still checked. The category could have
                // been un-marked as positionable in the meantime and the tag could have been
                // deselected, without triggering positionableTagDeselected and the area thus
                // still remaining. If the category is then re-marked as positionable, the area would
                // show up without the tag being selected.
                if(_listSelectList[category]->tagIsChecked(tag)) {
                    _preview->preview()->createTaggedArea(category, tag, areaData.value(), _preview->showAreas());
                }
            }
        }
    }

    if ( _setup == InputSingleImageConfigMode )
        setWindowTitle( i18n("KPhotoAlbum Annotations (%1/%2)", _current+1, _origList.count() ) );
    _preview->canCreateAreas( _setup == InputSingleImageConfigMode && ! info.isVideo() && _positionableCategories );
}

void AnnotationDialog::Dialog::writeToInfo()
{
    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
        (*it)->slotReturn();
    }

    DB::ImageInfo& info = _editList[ _current ];

    if (! info.size().isValid()) {
        // The actual image size has been fetched by ImagePreview, so we can add it to
        // the database silenty, so that it's saved if the database will be saved.
        info.setSize(_preview->preview()->getActualImageSize());
    }

    if ( _time->isHidden() ) {
        if ( _endDate->date().isValid() )
            info.setDate( DB::ImageDate( QDateTime( _startDate->date(), QTime(0,0,0) ),
                                     QDateTime( _endDate->date(), QTime( 23,59,59) ) ) );
        else
            info.setDate( DB::ImageDate( QDateTime( _startDate->date(), QTime(0,0,0) ),
                                     QDateTime( _startDate->date(), QTime( 23,59,59) ) ) );
    }
    else
        info.setDate( DB::ImageDate( QDateTime( _startDate->date(), _time->time() ) ) );

    // Generate a list of all tagged areas

    QMap<QString, QMap<QString, QRect>> taggedAreas;
    QPair<QString, QString> tagData;

    foreach (ResizableFrame *area, _preview->preview()->findChildren<ResizableFrame *>()) {
        tagData = area->tagData();

        if ( !tagData.first.isEmpty() ) {
            taggedAreas[tagData.first][tagData.second] = area->actualCoordinates();
        }
    }

    info.setLabel( _imageLabel->text() );
    info.setDescription( _description->toPlainText() );

    for (QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it) {
        info.setCategoryInfo( (*it)->category(), (*it)->itemsOn() );
        if ((*it)->positionable()) {
            info.setPositionedTags((*it)->category(), taggedAreas[(*it)->category()]);
        }
    }

    if ( _ratingChanged ) {
        info.setRating( _rating->rating() );
        _ratingChanged = false;
    }
}

void AnnotationDialog::Dialog::ShowHideSearch( bool show )
{
    _megapixel->setVisible( show );
    _megapixelLabel->setVisible( show );
    _searchRAW->setVisible( show );
    _imageFilePatternLabel->setVisible( show );
    _imageFilePattern->setVisible( show );
    _isFuzzyDate->setChecked( show );
    _isFuzzyDate->setVisible( !show );
    slotSetFuzzyDate();
    _ratingSearchMode->setVisible( show );
    _ratingSearchLabel->setVisible( show );
}


int AnnotationDialog::Dialog::configure( DB::ImageInfoList list, bool oneAtATime )
{
    ShowHideSearch(false);

    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        DB::ImageDB::instance()->categoryCollection()->categoryForName( Settings::SettingsData::instance()->untaggedCategory() )
            ->addItem(Settings::SettingsData::instance()->untaggedTag() );
    }

    if ( oneAtATime )
        _setup = InputSingleImageConfigMode;
    else
        _setup = InputMultiImageConfigMode;

    _origList = list;
    _editList.clear();

    for( DB::ImageInfoListConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        _editList.append( *(*it) );
    }

    setup();

    if ( oneAtATime )  {
        _current = 0;
        _preview->configure( &_editList, true );
        load();
    }
    else {
        _preview->configure( &_editList, false );
        _preview->canCreateAreas( false );
        _startDate->setDate( QDate() );
        _endDate->setDate( QDate() );
        _time->hide();
        _rating->setRating( 0 );
        _ratingChanged = false;

        for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it )
            setUpCategoryListBoxForMultiImageSelection( *it, list );

        _imageLabel->setText(QString());
        _imageFilePattern->setText(QString());
        _firstDescription = _editList[0].description();

        const bool allTextEqual =
                std::all_of(_editList.begin(), _editList.end(),
                            [=] (const DB::ImageInfo& item) -> bool {
                                   return item.description() == _firstDescription;
                });

        if ( !allTextEqual )
            _firstDescription = conflictText;
        _description->setPlainText( _firstDescription );
    }

    showHelpDialog( oneAtATime ? InputSingleImageConfigMode : InputMultiImageConfigMode );

    return exec();
}

DB::ImageSearchInfo AnnotationDialog::Dialog::search( DB::ImageSearchInfo* search  )
{
    ShowHideSearch(true);

    _setup = SearchMode;
    if ( search )
        _oldSearch = *search;

    setup();

    _preview->setImage(Utilities::locateDataFile(QString::fromLatin1("pics/search.jpg")));

    _ratingChanged = false ;
    showHelpDialog( SearchMode );
    int ok = exec();
    if ( ok == QDialog::Accepted )  {
        const QDateTime start = _startDate->date().isNull() ? QDateTime() : QDateTime(_startDate->date());
        const QDateTime end = _endDate->date().isNull() ? QDateTime() : QDateTime( _endDate->date() );
        _oldSearch = DB::ImageSearchInfo( DB::ImageDate( start, end ),
                      _imageLabel->text(), _description->toPlainText(),
                      _imageFilePattern->text());

        for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
            _oldSearch.setCategoryMatchText( (*it)->category(), (*it)->text() );
        }
        //FIXME: for the user to search for 0-rated images, he must first change the rating to anything > 0
        //then change back to 0 .
        if( _ratingChanged)
          _oldSearch.setRating( _rating->rating() );

        _ratingChanged = false;
        _oldSearch.setSearchMode( _ratingSearchMode->currentIndex() );
        _oldSearch.setMegaPixel( _megapixel->value() );
        _oldSearch.setSearchRAW( _searchRAW->isChecked() );
        return _oldSearch;
    }
    else
    return DB::ImageSearchInfo();
}

void AnnotationDialog::Dialog::setup()
{
// Repopulate the listboxes in case data has changed
    // An group might for example have been renamed.
    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
        (*it)->populate();
    }

    if ( _setup == SearchMode )  {
        _okBut->setGuiItem( KGuiItem(i18nc("@action:button","&Search"), QString::fromLatin1("find")) );
        _continueLaterBut->hide();
        _revertBut->hide();
        _clearBut->show();
        setWindowTitle( i18nc("@title:window title of the 'find images' window","Search") );
        loadInfo( _oldSearch );
    }
    else {
        _okBut->setText( i18n("Done") );
        _continueLaterBut->show();
        _revertBut->setEnabled( _setup == InputSingleImageConfigMode );
        _clearBut->hide();
        _revertBut->show();
        setWindowTitle( i18n("Annotations") );
    }

    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it )
        (*it)->setMode( _setup );
}


void AnnotationDialog::Dialog::slotClear()
{
    loadInfo( DB::ImageSearchInfo() );
}

void AnnotationDialog::Dialog::loadInfo( const DB::ImageSearchInfo& info )
{
    _startDate->setDate( info.date().start().date() );
    _endDate->setDate( info.date().end().date() );

    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
        (*it)->setText( info.categoryMatchText( (*it)->category() ) );
    }

    _imageLabel->setText( info.label() );
    _description->setText(info.description());
}

void AnnotationDialog::Dialog::slotOptions()
{
    // create menu entries for dock windows
    QMenu* menu = new QMenu( this );
    QMenu* dockMenu =_dockWindow->createPopupMenu();
    menu->addMenu( dockMenu )
        ->setText( i18n( "Configure window layout..." ) );
    QAction* saveCurrent = dockMenu->addAction( i18n("Save Current Window Setup") );
    QAction* reset = dockMenu->addAction( i18n( "Reset layout" ) );

    // create SortType entries
    menu->addSeparator();
    QActionGroup* sortTypes = new QActionGroup( menu );
    QAction* alphaTreeSort = new QAction(
            SmallIcon( QString::fromLatin1( "view-list-tree" ) ),
            i18n("Sort Alphabetically (Tree)"),
            sortTypes );
    QAction* alphaFlatSort = new QAction(
            SmallIcon( QString::fromLatin1( "draw-text" ) ),
            i18n("Sort Alphabetically (Flat)"),
            sortTypes );
    QAction* dateSort = new QAction(
            SmallIcon( QString::fromLatin1( "x-office-calendar" ) ),
            i18n("Sort by date"),
            sortTypes );
    alphaTreeSort->setCheckable( true );
    alphaFlatSort->setCheckable( true );
    dateSort->setCheckable( true );
    alphaTreeSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree );
    alphaFlatSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat );
    dateSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse );
    menu->addActions( sortTypes->actions() );
    connect( dateSort, SIGNAL(triggered()), _optionList.at(0), SLOT(slotSortDate()) );
    connect( alphaTreeSort, SIGNAL(triggered()), _optionList.at(0), SLOT(slotSortAlphaTree()) );
    connect( alphaFlatSort, SIGNAL(triggered()), _optionList.at(0), SLOT(slotSortAlphaFlat()) );

    // create MatchType entries
    menu->addSeparator();
    QActionGroup* matchTypes = new QActionGroup( menu );
    QAction* matchFromBeginning = new QAction( i18n( "Match tags from the first character."), matchTypes );
    QAction* matchFromWordStart = new QAction( i18n( "Match tags from word boundaries." ), matchTypes );
    QAction* matchAnywhere = new QAction( i18n( "Match tags anywhere."),matchTypes );
    matchFromBeginning->setCheckable( true );
    matchFromWordStart->setCheckable( true );
    matchAnywhere->setCheckable( true );
    // TODO add StatusTip text?
    // set current state:
    matchFromBeginning->setChecked( Settings::SettingsData::instance()->matchType() == AnnotationDialog::MatchFromBeginning );
    matchFromWordStart->setChecked( Settings::SettingsData::instance()->matchType() == AnnotationDialog::MatchFromWordStart );
    matchAnywhere->setChecked( Settings::SettingsData::instance()->matchType() == AnnotationDialog::MatchAnywhere );
    // add MatchType actions to menu:
    menu->addActions( matchTypes->actions() );

    // create toggle-show-selected entry#
    if ( _setup != SearchMode )
    {
        menu->addSeparator();
        QAction* showSelectedOnly = new QAction(
                SmallIcon( QString::fromLatin1( "view-filter" ) ),
                i18n("Show only selected Ctrl+S"),
                menu );
        showSelectedOnly->setCheckable( true );
        showSelectedOnly->setChecked( ShowSelectionOnlyManager::instance().selectionIsLimited() );
        menu->addAction( showSelectedOnly );

        connect( showSelectedOnly, SIGNAL(triggered()), &ShowSelectionOnlyManager::instance(), SLOT(toggle()) );
    }

    // execute menu & handle response:
    QAction* res = menu->exec( QCursor::pos() );
    if ( res == saveCurrent )
        slotSaveWindowSetup();
    else if ( res == reset )
        slotResetLayout();
    else if ( res == matchFromBeginning )
        Settings::SettingsData::instance()->setMatchType( AnnotationDialog::MatchFromBeginning );
    else if ( res == matchFromWordStart )
        Settings::SettingsData::instance()->setMatchType( AnnotationDialog::MatchFromWordStart );
    else if ( res == matchAnywhere )
        Settings::SettingsData::instance()->setMatchType( AnnotationDialog::MatchAnywhere );
}

int AnnotationDialog::Dialog::exec()
{
    _stack->setCurrentWidget( _dockWindow );
    showTornOfWindows();
    this->setFocus(); // Set temporary focus before show() is called so that extra cursor is not shown on any "random" input widget
    show(); // We need to call show before we call setupFocus() otherwise the widget will not yet all have been moved in place.
    setupFocus();
    const int ret = KDialog::exec();
    hideTornOfWindows();
    return ret;
}

void AnnotationDialog::Dialog::slotSaveWindowSetup()
{
    const QByteArray data = _dockWindow->saveState();

    QFile file( QString::fromLatin1( "%1/layout.dat" ).arg( Settings::SettingsData::instance()->imageDirectory() ) );
    if ( !file.open( QIODevice::WriteOnly ) ) {
        KMessageBox::sorry( this,
                i18n("<p>Could not save the window layout.</p>"
                    "File %1 could not be opened because of the following error: %2"
                    , file.fileName(), file.errorString() )
                );
    } else if ( ! ( file.write( data ) && file.flush() ) )
    {
        KMessageBox::sorry( this,
                i18n("<p>Could not save the window layout.</p>"
                    "File %1 could not be written because of the following error: %2"
                    , file.fileName(), file.errorString() )
                );
    }
    file.close();
}

void AnnotationDialog::Dialog::closeEvent( QCloseEvent* e )
{
    e->ignore();
    reject();
}

void AnnotationDialog::Dialog::hideTornOfWindows()
{
    for( QDockWidget* dock : _dockWidgets ) {
        if ( dock->isFloating() )
            dock->hide();
    }
}

void AnnotationDialog::Dialog::showTornOfWindows()
{
    for (QDockWidget* dock: _dockWidgets ) {
        if ( dock->isFloating() )
            dock->show();
    }
}


AnnotationDialog::ListSelect* AnnotationDialog::Dialog::createListSel( const DB::CategoryPtr& category )
{
    ListSelect* sel = new ListSelect( category, _dockWindow );
    _optionList.append( sel );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL(itemRemoved(DB::Category*,QString)),
             this, SLOT(slotDeleteOption(DB::Category*,QString)) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL(itemRenamed(DB::Category*,QString,QString)),
             this, SLOT(slotRenameOption(DB::Category*,QString,QString)) );

    return sel;
}

void AnnotationDialog::Dialog::slotDeleteOption( DB::Category* category, const QString& value )
{
    for( QList<DB::ImageInfo>::Iterator it = _editList.begin(); it != _editList.end(); ++it ) {
        (*it).removeCategoryInfo( category->name(), value );
    }
}

void AnnotationDialog::Dialog::slotRenameOption( DB::Category* category, const QString& oldValue, const QString& newValue )
{
    for( QList<DB::ImageInfo>::Iterator it = _editList.begin(); it != _editList.end(); ++it ) {
        (*it).renameItem( category->name(), oldValue, newValue );
    }
}

void AnnotationDialog::Dialog::reject()
{
    _fullScreenPreview->stopPlayback();
    if ( hasChanges() ) {
        int code =  KMessageBox::questionYesNo( this, i18n("<p>Some changes are made to annotations. Do you really want to cancel all recent changes for each affected file?</p>") );
        if ( code == KMessageBox::No )
            return;
    }
    closeDialog();
}

void AnnotationDialog::Dialog::closeDialog()
{
    tidyAreas();
    _accept = QDialog::Rejected;
    QDialog::reject();
}

bool AnnotationDialog::Dialog::hasChanges()
{
    bool changed = false;
    if ( _setup == InputSingleImageConfigMode )  {
        writeToInfo();
        for ( int i = 0; i < _editList.count(); ++i )  {
            changed |= (*(_origList[i]) != _editList[i]);
        }
    }

    else if ( _setup == InputMultiImageConfigMode ) {
        changed |= ( !_startDate->date().isNull() );
        changed |= ( !_endDate->date().isNull() );

        for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
            StringSet on, partialOn;
            std::tie(on, partialOn) = selectionForMultiSelect( *it, _origList );
            changed |= (on != (*it)->itemsOn());
            changed |= (partialOn != (*it)->itemsUnchanged());
        }

        changed |= ( !_imageLabel->text().isEmpty() );
        changed |= ( _description->toPlainText() != _firstDescription );
        changed |= _ratingChanged;
    }
    return changed;
}

void AnnotationDialog::Dialog::rotate( int angle )
{
    if ( _setup == InputMultiImageConfigMode ) {
        // In doneTagging the preview will be queried for its angle.
    }
    else {
        DB::ImageInfo& info = _editList[ _current ];
        info.rotate( angle, DB::RotateImageInfoOnly );
        emit imageRotated( info.fileName() );
    }
}

void AnnotationDialog::Dialog::slotSetFuzzyDate()
{
    if ( _isFuzzyDate->isChecked() )
    {
        _time->hide();
        _timeLabel->hide();
        _endDate->show();
        _endDateLabel->show();
    } else {
        _time->show();
        _timeLabel->show();
        _endDate->hide();
        _endDateLabel->hide();
    }
}

void AnnotationDialog::Dialog::slotDeleteImage()
{
    // CTRL+Del is a common key combination when editing text
    // TODO: The word right of cursor should be deleted as expected also in date and category fields
    if ( _setup == SearchMode )
    return;

    if( _setup == InputMultiImageConfigMode )  //TODO: probably delete here should mean remove from selection
      return;

    DB::ImageInfoPtr info = _origList[_current];

    _origList.remove( info );
    _editList.removeAll( _editList.at( _current ) );
    MainWindow::DirtyIndicator::markDirty();
    if ( _origList.count() == 0 ) {
        doneTagging();
        return;
    }
    if ( _current == (int)_origList.count() ) // we deleted the last image
        _current--;

    load();
}

void AnnotationDialog::Dialog::showHelpDialog( UsageMode type )
{
    QString doNotShowKey;
    QString txt;
    if ( type == SearchMode ) {
        doNotShowKey = QString::fromLatin1( "image_config_search_show_help" );
        txt = i18n( "<p>You have just opened the advanced search dialog; to get the most out of it, "
                    "it is suggested that you read the section in the manual on <a href=\"help:/kphotoalbum/sect-general-image-searches.html\">"
                    "advanced searching</a>.</p>"
                    "<p>This dialog is also used for typing in information about images; you can find "
                    "extra tips on its usage by reading about "
                    "<a href=\"help:/kphotoalbum/chp-typingIn.html\">typing in</a>.</p>" );
    }
    else {
        doNotShowKey = QString::fromLatin1( "image_config_typein_show_help" );
        txt = i18n("<p>You have just opened one of the most important windows in KPhotoAlbum; "
                   "it contains lots of functionality which has been optimized for fast usage.</p>"
                   "<p>It is strongly recommended that you take 5 minutes to read the "
                   "<a href=\"help:/kphotoalbum/chp-typingIn.html\">documentation for this "
                   "dialog</a></p>" );
    }


    KMessageBox::information( this, txt, QString(), doNotShowKey, KMessageBox::AllowLink );
}

void AnnotationDialog::Dialog::resizeEvent( QResizeEvent* )
{
    Settings::SettingsData::instance()->setWindowGeometry( Settings::AnnotationDialog, geometry() );
}

void AnnotationDialog::Dialog::moveEvent( QMoveEvent * )
{
    Settings::SettingsData::instance()->setWindowGeometry( Settings::AnnotationDialog, geometry() );

}


void AnnotationDialog::Dialog::setupFocus()
{
    QList<QWidget*> list = findChildren<QWidget*>();
    QList<QWidget*> orderedList;

    // Iterate through all widgets in our dialog.
    for ( QObject* obj : list ) {
        QWidget* current = static_cast<QWidget*>( obj );
        if ( !current->property("WantsFocus").isValid() || !current->isVisible() )
            continue;

        int cx = current->mapToGlobal( QPoint(0,0) ).x();
        int cy = current->mapToGlobal( QPoint(0,0) ).y();

        bool inserted = false;
        // Iterate through the ordered list of widgets, and insert the current one, so it is in the right position in the tab chain.
        for( QList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
            const QWidget* w = *orderedIt;
            int wx = w->mapToGlobal( QPoint(0,0) ).x();
            int wy = w->mapToGlobal( QPoint(0,0) ).y();

            if ( wy > cy || ( wy == cy && wx >= cx ) ) {
                orderedList.insert( orderedIt, current );
                inserted = true;
                break;
            }
        }
        if (!inserted)
            orderedList.append( current );
    }


    // now setup tab order.
    QWidget* prev = nullptr;
    QWidget* first = nullptr;
    for( QList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( prev ) {
            setTabOrder( prev, *orderedIt );
        } else {
            first = *orderedIt;
        }
        prev = *orderedIt;
    }

    if ( first ) {
        setTabOrder( prev, first );
    }


    // Finally set focus on the first list select
    for( QList<QWidget*>::Iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
        if ( (*orderedIt)->property("FocusCandidate").isValid() && (*orderedIt)->isVisible() ) {
            (*orderedIt)->setFocus();
            break;
        }
    }
}

void AnnotationDialog::Dialog::slotResetLayout()
{
    _dockWindow->restoreState(_dockWindowCleanState);
}

void AnnotationDialog::Dialog::slotStartDateChanged( const DB::ImageDate& date )
{
    if ( date.start() == date.end() )
        _endDate->setDate( QDate() );
    else
        _endDate->setDate( date.end().date() );
}

void AnnotationDialog::Dialog::loadWindowLayout()
{
    QString fileName =  QString::fromLatin1( "%1/layout.dat" ).arg( Settings::SettingsData::instance()->imageDirectory() );
    if ( !QFileInfo(fileName).exists() )
        return;

    QFile file( fileName );
    file.open( QIODevice::ReadOnly );
    QByteArray data = file.readAll();
    _dockWindow->restoreState(data);
}

void AnnotationDialog::Dialog::setupActions()
{
    _actions = new KActionCollection( this );

    KAction* action = nullptr;
    action = _actions->addAction( QString::fromLatin1("annotationdialog-sort-alphatree"), _optionList.at(0), SLOT(slotSortAlphaTree()) );
    action->setText( i18n("Sort Alphabetically (Tree)") );
    action->setShortcut(Qt::CTRL+Qt::Key_F4);

    action = _actions->addAction( QString::fromLatin1("annotationdialog-sort-alphaflat"), _optionList.at(0), SLOT(slotSortAlphaFlat()) );
    action->setText( i18n("Sort Alphabetically (Flat)") );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-sort-MRU"), _optionList.at(0), SLOT(slotSortDate()) );
    action->setText( i18n("Sort Most Recently Used") );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-toggle-sort"),  _optionList.at(0), SLOT(toggleSortType()) );
    action->setText( i18n("Toggle Sorting") );
    action->setShortcut( Qt::CTRL+Qt::Key_T );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-toggle-showing-selected-only"),
                                  &ShowSelectionOnlyManager::instance(), SLOT(toggle()) );
    action->setText( i18n("Toggle Showing Selected Items Only") );
    action->setShortcut( Qt::CTRL+Qt::Key_S );


    action = _actions->addAction( QString::fromLatin1("annotationdialog-next-image"),  _preview, SLOT(slotNext()) );
    action->setText(  i18n("Annotate Next") );
    action->setShortcut(  Qt::Key_PageDown );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-prev-image"),  _preview, SLOT(slotPrev()) );
    action->setText(  i18n("Annotate Previous") );
    action->setShortcut(  Qt::Key_PageUp );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-OK-dialog"),  this, SLOT(doneTagging()) );
    action->setText(  i18n("OK dialog") );
    action->setShortcut(  Qt::CTRL+Qt::Key_Return );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-delete-image"),  this, SLOT(slotDeleteImage()) );
    action->setText(  i18n("Delete") );
    action->setShortcut(  Qt::CTRL+Qt::Key_Delete );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-copy-previous"),  this, SLOT(slotCopyPrevious()) );
    action->setText(  i18n("Copy tags from previous image") );
    action->setShortcut(  Qt::ALT+Qt::Key_Insert );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-rotate-left"),  _preview, SLOT(rotateLeft()) );
    action->setText(  i18n("Rotate counterclockwise") );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-rotate-right"),  _preview, SLOT(rotateRight()) );
    action->setText(  i18n("Rotate clockwise") );

    action = _actions->addAction( QString::fromLatin1("annotationdialog-toggle-viewer"), this, SLOT(togglePreview()) );
    action->setText( i18n("Toggle fullscreen preview") );
    action->setShortcut( Qt::CTRL + Qt::Key_Space );

    foreach (QAction* action, _actions->actions()) {
      action->setShortcutContext(Qt::WindowShortcut);
      addAction(action);
  }
}

KActionCollection* AnnotationDialog::Dialog::actions()
{
    return _actions;
}

void AnnotationDialog::Dialog::setUpCategoryListBoxForMultiImageSelection( ListSelect* listSel, const DB::ImageInfoList& images )
{
    StringSet on, partialOn;
    std::tie(on,partialOn)  = selectionForMultiSelect( listSel, images );
    listSel->setSelection( on, partialOn );
}

std::tuple<StringSet,StringSet> AnnotationDialog::Dialog::selectionForMultiSelect( ListSelect* listSel, const DB::ImageInfoList& images )
{
    const QString category = listSel->category();
    const StringSet allItems = DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->itemsInclCategories().toSet();
    StringSet itemsNotSelectedOnAllImages;
    StringSet itemsOnSomeImages;

    for ( DB::ImageInfoList::ConstIterator imageIt = images.begin(); imageIt != images.end(); ++ imageIt ) {
        const StringSet itemsOnThisImage = (*imageIt)->itemsOfCategory( category );
        itemsNotSelectedOnAllImages += ( allItems - itemsOnThisImage );
        itemsOnSomeImages += itemsOnThisImage;
    }

    const StringSet itemsOnAllImages = allItems - itemsNotSelectedOnAllImages;
    const StringSet itemsPartiallyOn = itemsOnSomeImages - itemsOnAllImages;

    return std::make_tuple( itemsOnAllImages,  itemsPartiallyOn );
}

void AnnotationDialog::Dialog::slotRatingChanged( unsigned int )
{
    _ratingChanged = true;
}

void AnnotationDialog::Dialog::continueLater()
{
    saveAndClose();
}

void AnnotationDialog::Dialog::saveAndClose()
{
    tidyAreas();

    _fullScreenPreview->stopPlayback();

    if (_origList.isEmpty()) {
        // all images are deleted.
        QDialog::accept();
        return;
    }

    // I need to check for the changes first, as the case for _setup == InputSingleImageConfigMode, saves to the _origList,
    // and we can thus not check for changes anymore
    const bool anyChanges = hasChanges();

    if ( _setup == InputSingleImageConfigMode )  {
        writeToInfo();
        for ( int i = 0; i < _editList.count(); ++i )  {
            *(_origList[i]) = _editList[i];
        }
    }
    else if ( _setup == InputMultiImageConfigMode ) {
    for( QList<ListSelect*>::Iterator it = _optionList.begin(); it != _optionList.end(); ++it ) {
            (*it)->slotReturn();
        }

        for( DB::ImageInfoListConstIterator it = _origList.constBegin(); it != _origList.constEnd(); ++it ) {
            DB::ImageInfoPtr info = *it;
            info->delaySavingChanges(true);
            if ( !_startDate->date().isNull() )
                info->setDate( DB::ImageDate( _startDate->date(), _endDate->date(), _time->time() ) );

            for( QList<ListSelect*>::Iterator listSelectIt = _optionList.begin(); listSelectIt != _optionList.end(); ++listSelectIt ) {
                info->addCategoryInfo( (*listSelectIt)->category(), (*listSelectIt)->itemsOn() );
                info->removeCategoryInfo( (*listSelectIt)->category(), (*listSelectIt)->itemsOff() );
            }

            if ( !_imageLabel->text().isEmpty() ) {
                info->setLabel( _imageLabel->text() );
            }


            if ( !_description->toPlainText().isEmpty() && _description->toPlainText().compare( conflictText ) ) {
                info->setDescription( _description->toPlainText() );
            }

            if( _ratingChanged)
            {
              info->setRating( _rating->rating() );
            }

            info->delaySavingChanges(false);
        }
        _ratingChanged = false;
    }
    _accept = QDialog::Accepted;

    if ( anyChanges )
        MainWindow::DirtyIndicator::markDirty();

    QDialog::accept();
}

AnnotationDialog::Dialog::~Dialog()
{
    qDeleteAll( _optionList );
    _optionList.clear();
}

void AnnotationDialog::Dialog::togglePreview()
{
    if ( _stack->currentWidget() == _fullScreenPreview ) {
        _stack->setCurrentWidget( _dockWindow );
        _fullScreenPreview->stopPlayback();
    }
    else {
        _stack->setCurrentWidget( _fullScreenPreview );
        _fullScreenPreview->load( DB::FileNameList() << _editList[ _current].fileName() );
    }
}

void AnnotationDialog::Dialog::tidyAreas()
{
    // Remove all areas marked on the preview image
    foreach (ResizableFrame *area, _preview->preview()->findChildren<ResizableFrame *>()) {
        area->deleteLater();
    }
}

void AnnotationDialog::Dialog::slotNewArea(ResizableFrame *area)
{
    area->setDialog(this);
}

void AnnotationDialog::Dialog::positionableTagSelected(QString category, QString tag)
{
    // Set the selected tag as the last selected positionable tag
    _lastSelectedPositionableTag.first = category;
    _lastSelectedPositionableTag.second = tag;

    // Add the tag to the positionable tag candidate list
    addTagToCandidateList(category, tag);
}

void AnnotationDialog::Dialog::positionableTagDeselected(QString category, QString tag)
{
    // Remove the tag from the candidate list
    removeTagFromCandidateList(category, tag);

    // Search for areas linked against the tag on this image
    if (_setup == InputSingleImageConfigMode) {
        QPair<QString, QString> deselectedTag = QPair<QString, QString>(category, tag);

        QList<ResizableFrame *> allAreas = _preview->preview()->findChildren<ResizableFrame *>();
        foreach (ResizableFrame *area, allAreas) {
            if (area->tagData() == deselectedTag) {
                area->removeTagData();
                // Only one area can be associated with the tag, so we can return here
                return;
            }
        }
    }
    // Removal of tagged areas in InputMultiImageConfigMode is done in DB::ImageInfo::removeCategoryInfo
}

void AnnotationDialog::Dialog::addTagToCandidateList(QString category, QString tag)
{
    _positionableTagCandidates << QPair<QString, QString>(category, tag);
}

void AnnotationDialog::Dialog::removeTagFromCandidateList(QString category, QString tag)
{
    // Is the deselected tag the last selected positionable tag?
    if (_lastSelectedPositionableTag.first == category and _lastSelectedPositionableTag.second == tag) {
        _lastSelectedPositionableTag = QPair<QString, QString>();
    }

    // Remove the tag from the candidate list
    _positionableTagCandidates.removeAt(
        _positionableTagCandidates.indexOf(
            QPair<QString, QString>(category, tag)
        )
    );
}

QPair<QString, QString> AnnotationDialog::Dialog::lastSelectedPositionableTag() const
{
    return _lastSelectedPositionableTag;
}

QList<QPair<QString, QString>> AnnotationDialog::Dialog::positionableTagCandidates() const
{
    return _positionableTagCandidates;
}

void AnnotationDialog::Dialog::slotShowAreas(bool showAreas)
{
    QList<ResizableFrame *> allAreas = _preview->preview()->findChildren<ResizableFrame *>();
    foreach (ResizableFrame *area, allAreas) {
        area->setVisible(showAreas);
    }
}

void AnnotationDialog::Dialog::positionableTagRenamed(QString category, QString oldTag, QString newTag)
{
    // Is the renamed tag the last selected positionable tag?
    if (_lastSelectedPositionableTag.first == category and _lastSelectedPositionableTag.second == oldTag) {
        _lastSelectedPositionableTag.second = newTag;
    }

    // Check the candidate list for the tag
    QPair<QString, QString> oldTagData = QPair<QString, QString>(category, oldTag);
    if (_positionableTagCandidates.contains(oldTagData)) {
        // The tag is in the list, so update it
        _positionableTagCandidates.removeAt(_positionableTagCandidates.indexOf(oldTagData));
        _positionableTagCandidates << QPair<QString, QString>(category, newTag);
    }

    // Check if an area on the current image contains the changed tag
    QList<ResizableFrame *> allAreas = _preview->preview()->findChildren<ResizableFrame *>();
    foreach (ResizableFrame *area, allAreas) {
        if (area->tagData() == oldTagData) {
            area->setTagData(category, newTag);
            // Only one area can contain the tag, so we can break here.
            break;
        }
    }
}

QString AnnotationDialog::Dialog::localizedCategory(QString category) const
{
    if (_categoryL10n.contains(category)) {
        return _categoryL10n[category];
    } else {
        return category;
    }
}

void AnnotationDialog::Dialog::descriptionPageUpDownPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp) {
        _actions->action(QString::fromLatin1("annotationdialog-prev-image"))->trigger();
    } else if (event->key() == Qt::Key_PageDown) {
        _actions->action(QString::fromLatin1("annotationdialog-next-image"))->trigger();
    }
}

#include "Dialog.moc"

// vi:expandtab:tabstop=4 shiftwidth=4:
