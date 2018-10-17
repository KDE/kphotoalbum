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

#include "Dialog.h"

#include "DescriptionEdit.h"
#include "enums.h"
#include "ImagePreviewWidget.h"
#include "DateEdit.h"
#include "ListSelect.h"
#include "Logging.h"
#include "ResizableFrame.h"
#include "ShortCutManager.h"
#include "ShowSelectionOnlyManager.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <MainWindow/DirtyIndicator.h>
#include <Settings/SettingsData.h>
#include <Utilities/ShowBusyCursor.h>
#include <Utilities/Util.h>
#include <Viewer/ViewerWidget.h>

#include <KAcceleratorManager>
#include <KActionCollection>
#include <KComboBox>
#include <KGuiItem>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRatingWidget>
#include <KTextEdit>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QCursor>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimeEdit>
#include <QVBoxLayout>
#ifdef HAVE_KGEOMAP
#include <Map/MapView.h>
#include <QProgressBar>
#include <QTimer>
#endif

#include <algorithm>
#include <tuple>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QtGlobal>

using Utilities::StringSet;

/**
 * \class AnnotationDialog::Dialog
 * \brief QDialog subclass used for tagging images
 */

AnnotationDialog::Dialog::Dialog( QWidget* parent )
    : QDialog( parent )
    , m_ratingChanged( false )
    , m_conflictText( i18n("(You have differing descriptions on individual images, setting text here will override them all)" ) )
{
    Utilities::ShowBusyCursor dummy;
    ShortCutManager shortCutManager;

    // The widget stack
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(mainWidget);
    setLayout(layout);
    layout->addWidget(mainWidget);
    m_stack = new QStackedWidget(mainWidget);
    layout->addWidget( m_stack );

    // The Viewer
    m_fullScreenPreview = new Viewer::ViewerWidget( Viewer::ViewerWidget::InlineViewer );
    m_stack->addWidget( m_fullScreenPreview );

    // The dock widget
    m_dockWindow = new QMainWindow;
    m_stack->addWidget( m_dockWindow );
    m_dockWindow->setDockNestingEnabled( true );

    // -------------------------------------------------- Dock widgets
    m_generalDock = createDock( i18n("Label and Dates"), QString::fromLatin1("Label and Dates"), Qt::TopDockWidgetArea, createDateWidget(shortCutManager) );

    m_previewDock = createDock( i18n("Image Preview"), QString::fromLatin1("Image Preview"), Qt::TopDockWidgetArea, createPreviewWidget() );

    m_description = new DescriptionEdit(this);
    m_description->setProperty( "WantsFocus", true );
    m_description->setObjectName( i18n("Description") );
    m_description->setCheckSpellingEnabled( true );
    m_description->setTabChangesFocus( true ); // this allows tabbing to the next item in the tab order.
    m_description->setWhatsThis( i18nc( "@info:whatsthis",
                "<para>A descriptive text of the image.</para>"
                "<para>If <emphasis>Use Exif description</emphasis> is enabled under "
                "<interface>Settings|Configure KPhotoAlbum...|General</interface>, a description "
                "embedded in the image Exif information is imported to this field if available.</para>"
                ));

    m_descriptionDock = createDock( i18n("Description"), QString::fromLatin1("description"), Qt::LeftDockWidgetArea, m_description );
    shortCutManager.addDock( m_descriptionDock, m_description );

    connect( m_description, SIGNAL(pageUpDownPressed(QKeyEvent*)), this, SLOT(descriptionPageUpDownPressed(QKeyEvent*)) );

#ifdef HAVE_KGEOMAP
    // -------------------------------------------------- Map representation

    m_annotationMapContainer = new QWidget(this);
    QVBoxLayout *annotationMapContainerLayout = new QVBoxLayout(m_annotationMapContainer);

    m_annotationMap = new Map::MapView(this);
    annotationMapContainerLayout->addWidget(m_annotationMap);

    QHBoxLayout *mapLoadingProgressLayout = new QHBoxLayout();
    annotationMapContainerLayout->addLayout(mapLoadingProgressLayout);

    m_mapLoadingProgress = new QProgressBar(this);
    mapLoadingProgressLayout->addWidget(m_mapLoadingProgress);
    m_mapLoadingProgress->hide();

    m_cancelMapLoadingButton = new QPushButton(i18n("Cancel"));
    mapLoadingProgressLayout->addWidget(m_cancelMapLoadingButton);
    m_cancelMapLoadingButton->hide();
    connect(m_cancelMapLoadingButton, SIGNAL(clicked()), this, SLOT(setCancelMapLoading()));

    m_annotationMapContainer->setObjectName(i18n("Map"));
    m_mapDock = createDock(
        i18n("Map"),
        QString::fromLatin1("map"),
        Qt::LeftDockWidgetArea,
        m_annotationMapContainer
    );
    shortCutManager.addDock(m_mapDock, m_annotationMapContainer);
    connect(m_mapDock, SIGNAL(visibilityChanged(bool)), this, SLOT(annotationMapVisibilityChanged(bool)));
    m_mapDock->setWhatsThis( i18nc( "@info:whatsthis", "The map widget allows you to view the location of images if GPS coordinates are found in the Exif information." ));
#endif

    // -------------------------------------------------- Categories
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();

    // Let's first assume we don't have positionable categories
    m_positionableCategories = false;

    for( QList<DB::CategoryPtr>::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt ) {
        ListSelect* sel = createListSel( *categoryIt );

        // Create a QMap of all ListSelect instances, so that we can easily
        // check if a specific (positioned) tag is (still) selected later
        m_listSelectList[(*categoryIt)->name()] = sel;

        QDockWidget* dock = createDock((*categoryIt)->name(),
                                       (*categoryIt)->name(),
                                       Qt::BottomDockWidgetArea,
                                       sel);
        shortCutManager.addDock( dock, sel->lineEdit() );

        if ( (*categoryIt)->isSpecialCategory() )
            dock->hide();

        // Pass the positionable selection to the object
        sel->setPositionable( (*categoryIt)->positionable() );

        if ( sel->positionable() ) {
            connect( sel, SIGNAL(positionableTagSelected(QString,QString)), this, SLOT(positionableTagSelected(QString,QString)) );
            connect( sel, SIGNAL(positionableTagDeselected(QString,QString)), this, SLOT(positionableTagDeselected(QString,QString)) );
            connect( sel, SIGNAL(positionableTagRenamed(QString,QString,QString)), this, SLOT(positionableTagRenamed(QString,QString,QString)) );

            connect(m_preview->preview(), SIGNAL(proposedTagSelected(QString,QString)), sel, SLOT(ensureTagIsSelected(QString,QString)));

            // We have at least one positionable category
            m_positionableCategories = true;
        }
    }

    // -------------------------------------------------- The buttons.
    // don't use default buttons (Ok, Cancel):
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::NoButton);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QHBoxLayout* lay1 = new QHBoxLayout;
    layout->addLayout( lay1 );

    m_revertBut = new QPushButton( i18n("Revert This Item") );
    KAcceleratorManager::setNoAccel(m_revertBut);
    lay1->addWidget( m_revertBut );

    m_clearBut = new QPushButton();
    KGuiItem::assign(m_clearBut,
                     KGuiItem( i18n("Clear Form"),QApplication::isRightToLeft()
                               ? QString::fromLatin1("clear_left")
                               : QString::fromLatin1("locationbar_erase")) ) ;
    KAcceleratorManager::setNoAccel(m_clearBut);
    lay1->addWidget( m_clearBut );

    QPushButton* optionsBut = new QPushButton( i18n("Options..." ) );
    KAcceleratorManager::setNoAccel(optionsBut);
    lay1->addWidget( optionsBut );

    lay1->addStretch(1);

    m_okBut = new QPushButton( i18n("&Done") );
    lay1->addWidget( m_okBut );

    m_continueLaterBut = new QPushButton( i18n("Continue &Later") );
    lay1->addWidget( m_continueLaterBut );

    QPushButton* cancelBut = new QPushButton();
    KGuiItem::assign( cancelBut, KStandardGuiItem::cancel() );
    lay1->addWidget( cancelBut );

    // It is unfortunately not possible to ask KAcceleratorManager not to setup the OK and cancel keys.
    shortCutManager.addTaken( i18nc("@action:button","&Search") );
    shortCutManager.addTaken( m_okBut->text() );
    shortCutManager.addTaken( m_continueLaterBut->text());
    shortCutManager.addTaken( cancelBut->text() );

    connect( m_revertBut, SIGNAL(clicked()), this, SLOT(slotRevert()) );
    connect( m_okBut, SIGNAL(clicked()), this, SLOT(doneTagging()) );
    connect( m_continueLaterBut, SIGNAL(clicked()), this, SLOT(continueLater()) );
    connect( cancelBut, SIGNAL(clicked()), this, SLOT(reject()) );
    connect( m_clearBut, SIGNAL(clicked()), this, SLOT(slotClear()) );
    connect( optionsBut, SIGNAL(clicked()), this, SLOT(slotOptions()) );

    connect( m_preview, SIGNAL(imageRotated(int)), this, SLOT(rotate(int)) );
    connect( m_preview, SIGNAL(indexChanged(int)), this, SLOT(slotIndexChanged(int)) );
    connect( m_preview, SIGNAL(imageDeleted(DB::ImageInfo)), this, SLOT(slotDeleteImage()) );
    connect( m_preview, SIGNAL(copyPrevClicked()), this, SLOT(slotCopyPrevious()) );
    connect( m_preview, SIGNAL(areaVisibilityChanged(bool)), this, SLOT(slotShowAreas(bool)) );
    connect( m_preview->preview(), SIGNAL(areaCreated(ResizableFrame*)), this, SLOT(slotNewArea(ResizableFrame*)) );

    // Disable so no button accept return (which would break with the line edits)
    m_revertBut->setAutoDefault( false );
    m_okBut->setAutoDefault( false );
    m_continueLaterBut->setAutoDefault( false );
    cancelBut->setAutoDefault( false );
    m_clearBut->setAutoDefault( false );
    optionsBut->setAutoDefault( false );

    m_dockWindowCleanState = m_dockWindow->saveState();

    loadWindowLayout();

    m_current = -1;

    setGeometry( Settings::SettingsData::instance()->windowGeometry( Settings::AnnotationDialog ) );

    setupActions();
    shortCutManager.setupShortCuts();

    // WARNING layout->addWidget(buttonBox) must be last item in layout
    layout->addWidget(buttonBox);
}

QDockWidget* AnnotationDialog::Dialog::createDock( const QString& title, const QString& name,
                                                   Qt::DockWidgetArea location, QWidget* widget )
{
    QDockWidget* dock = new QDockWidget( title );
    KAcceleratorManager::setNoAccel(dock);
    dock->setObjectName( name );
    dock->setAllowedAreas( Qt::AllDockWidgetAreas );
    dock->setWidget( widget );
    m_dockWindow->addDockWidget( location, dock );
    m_dockWidgets.append( dock );
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
    m_imageLabel = new KLineEdit;
    m_imageLabel->setProperty( "WantsFocus", true );
    m_imageLabel->setObjectName( i18n("Label") );
    lay3->addWidget( m_imageLabel );
    shortCutManager.addLabel( label );
    label->setBuddy( m_imageLabel );


    // Date
    QHBoxLayout* lay4 = new QHBoxLayout;
    lay2->addLayout( lay4 );

    label = new QLabel( i18n("Date: ") );
    lay4->addWidget( label );

    m_startDate = new ::AnnotationDialog::DateEdit( true );
    lay4->addWidget( m_startDate, 1 );
    connect( m_startDate, SIGNAL(dateChanged(DB::ImageDate)), this, SLOT(slotStartDateChanged(DB::ImageDate)) );
    shortCutManager.addLabel(label );
    label->setBuddy( m_startDate);

    m_endDateLabel = new QLabel( QString::fromLatin1( "-" ) );
    lay4->addWidget( m_endDateLabel );

    m_endDate = new ::AnnotationDialog::DateEdit( false );
    lay4->addWidget( m_endDate, 1 );

    // Time
    m_timeLabel = new QLabel( i18n("Time: ") );
    lay4->addWidget( m_timeLabel );

    m_time= new QTimeEdit;
    lay4->addWidget( m_time );

    m_isFuzzyDate = new QCheckBox( i18n("Use Fuzzy Date") );
    m_isFuzzyDate->setWhatsThis( i18nc("@info",
                "<para>In KPhotoAlbum, images can either have an exact date and time"
                ", or a <emphasis>fuzzy</emphasis> date which happened any time during"
                " a specified time interval. Images produced by digital cameras"
                " do normally have an exact date.</para>"
                "<para>If you don't know exactly when a photo was taken"
                " (e.g. if the photo comes from an analog camera), then you should set"
                " <interface>Use Fuzzy Date</interface>.</para>") );
    m_isFuzzyDate->setToolTip( m_isFuzzyDate->whatsThis() );
    lay4->addWidget( m_isFuzzyDate );
    lay4->addStretch(1);
    connect(m_isFuzzyDate,SIGNAL(stateChanged(int)),this,SLOT(slotSetFuzzyDate()));

    QHBoxLayout* lay8 = new QHBoxLayout;
    lay2->addLayout( lay8 );

    m_megapixelLabel = new QLabel( i18n("Minimum megapixels:") );
    lay8->addWidget( m_megapixelLabel );

    m_megapixel = new QSpinBox;
    m_megapixel->setRange( 0, 99 );
    m_megapixel->setSingleStep( 1 );
    m_megapixelLabel->setBuddy( m_megapixel );
    lay8->addWidget( m_megapixel );
    lay8->addStretch( 1 );

    m_max_megapixelLabel = new QLabel( i18n("Maximum megapixels:") );
    lay8->addWidget( m_max_megapixelLabel );

    m_max_megapixel = new QSpinBox;
    m_max_megapixel->setRange( 0, 99 );
    m_max_megapixel->setSingleStep( 1 );
    m_max_megapixelLabel->setBuddy( m_max_megapixel );
    lay8->addWidget( m_max_megapixel );
    lay8->addStretch( 1 );

    QHBoxLayout* lay9 = new QHBoxLayout;
    lay2->addLayout( lay9 );

    label = new QLabel( i18n("Rating:") );
    lay9->addWidget( label );
    m_rating = new KRatingWidget;
    m_rating->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    lay9->addWidget( m_rating, 0, Qt::AlignCenter );
    connect( m_rating, SIGNAL(ratingChanged(uint)), this, SLOT(slotRatingChanged(uint)) );

    m_ratingSearchLabel = new QLabel( i18n("Rating search mode:") );
    lay9->addWidget( m_ratingSearchLabel );

    m_ratingSearchMode = new KComboBox( lay9 );
    m_ratingSearchMode->addItems( QStringList() << i18n("==") << i18n(">=") << i18n("<=") << i18n("!=") );
    m_ratingSearchLabel->setBuddy( m_ratingSearchMode );
    lay9->addWidget( m_ratingSearchMode );

    // File name search pattern
    QHBoxLayout* lay10 = new QHBoxLayout;
    lay2->addLayout( lay10 );

    m_imageFilePatternLabel = new QLabel( i18n("File Name Pattern: " ) );
    lay10->addWidget( m_imageFilePatternLabel );
    m_imageFilePattern = new KLineEdit;
    m_imageFilePattern->setObjectName( i18n("File Name Pattern") );
    lay10->addWidget( m_imageFilePattern );
    shortCutManager.addLabel( m_imageFilePatternLabel );
    m_imageFilePatternLabel->setBuddy( m_imageFilePattern );

    m_searchRAW = new QCheckBox( i18n("Search only for RAW files") );
    lay2->addWidget( m_searchRAW );

    lay9->addStretch( 1 );
    lay2->addStretch(1);

    return top;
}

QWidget* AnnotationDialog::Dialog::createPreviewWidget()
{
    m_preview = new ImagePreviewWidget();
    connect(m_preview, &ImagePreviewWidget::togglePreview, this, &Dialog::togglePreview);
    return m_preview;
}

void AnnotationDialog::Dialog::slotRevert()
{
    if ( m_setup == InputSingleImageConfigMode )
        load();
}

void AnnotationDialog::Dialog::slotIndexChanged( int index )
{
  if ( m_setup != InputSingleImageConfigMode )
        return;

    if(m_current >= 0 )
      writeToInfo();

    m_current = index;

    load();
}

void AnnotationDialog::Dialog::doneTagging()
{
    saveAndClose();
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        for( DB::ImageInfoListIterator it = m_origList.begin(); it != m_origList.end(); ++it ) {
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
    if ( m_setup != InputSingleImageConfigMode )
        return;
    if ( m_current < 1 )
        return;

    // FIXME: it would be better to compute the "previous image" in a better way, but let's stick with this for now...
    DB::ImageInfo& old_info = m_editList[ m_current - 1 ];

    m_positionableTagCandidates.clear();
    m_lastSelectedPositionableTag.first = QString();
    m_lastSelectedPositionableTag.second = QString();

    Q_FOREACH( ListSelect *ls, m_optionList ) {
        ls->setSelection( old_info.itemsOfCategory( ls->category() ) );

        // Also set all positionable tag candidates

        if ( ls->positionable() ) {
            QString category = ls->category();
            QSet<QString> selectedTags = old_info.itemsOfCategory( category );
            QSet<QString> positionedTagSet = positionedTags( category );

            // Add the tag to the positionable candiate list, if no area is already associated with it
            Q_FOREACH(const auto &tag, selectedTags)
            {
                if (!positionedTagSet.contains(tag))
                {
                    addTagToCandidateList(category, tag);
                }
            }

            // Check all areas for a linked tag in this category that is probably not selected anymore
            for(ResizableFrame *area : areas()) {
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

    // No areas have been changed
    m_areasChanged = false;

    // Empty the positionable tag candidate list and the last selected positionable tag
    m_positionableTagCandidates.clear();
    m_lastSelectedPositionableTag = QPair<QString, QString>();


    DB::ImageInfo& info = m_editList[ m_current ];
    m_startDate->setDate( info.date().start().date() );

    if( info.date().hasValidTime() ) {
        m_time->show();
        m_time->setTime( info.date().start().time());
        m_isFuzzyDate->setChecked(false);
    }
    else {
        m_time->hide();
        m_isFuzzyDate->setChecked(true);
    }

    if ( info.date().start().date() == info.date().end().date() )
        m_endDate->setDate( QDate() );
    else
        m_endDate->setDate( info.date().end().date() );

    m_imageLabel->setText( info.label() );
    m_description->setPlainText( info.description() );

    if ( m_setup == InputSingleImageConfigMode )
        m_rating->setRating( qMax( static_cast<short int>(0), info.rating() ) );
    m_ratingChanged = false;

    // A category areas have been linked against could have been deleted
    // or un-marked as positionable in the meantime, so ...
    QMap<QString, bool> categoryIsPositionable;

    QList<QString> positionableCategories;

    Q_FOREACH( ListSelect *ls, m_optionList ) {
        ls->setSelection( info.itemsOfCategory( ls->category() ) );
        ls->rePopulate();

        // Get all selected positionable tags and add them to the candidate list
        if (ls->positionable()) {
            QSet<QString> selectedTags = ls->itemsOn();

            Q_FOREACH( const QString &tagName, selectedTags ) {
                addTagToCandidateList( ls->category(), tagName );
            }
        }

        // ... create a list of all categories and their positionability ...
        categoryIsPositionable[ls->category()] = ls->positionable();

        if (ls->positionable()) {
            positionableCategories << ls->category();
        }
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
                if(m_listSelectList[category]->tagIsChecked(tag)) {
                    m_preview->preview()->createTaggedArea(category, tag, areaData.value(), m_preview->showAreas());
                }
            }
        }
    }

    if (m_setup == InputSingleImageConfigMode) {
        setWindowTitle(i18nc("@title:window image %1 of %2 images", "Annotations (%1/%2)",
                             m_current + 1,
                             m_origList.count()));
        m_preview->canCreateAreas(
            m_setup == InputSingleImageConfigMode && ! info.isVideo() && m_positionableCategories
        );
#ifdef HAVE_KGEOMAP
        updateMapForCurrentImage();
#endif
    }

    m_preview->updatePositionableCategories(positionableCategories);
}

void AnnotationDialog::Dialog::writeToInfo()
{
    Q_FOREACH( ListSelect *ls, m_optionList ) {
        ls->slotReturn();
    }

    DB::ImageInfo& info = m_editList[ m_current ];

    if (! info.size().isValid()) {
        // The actual image size has been fetched by ImagePreview, so we can add it to
        // the database silenty, so that it's saved if the database will be saved.
        info.setSize(m_preview->preview()->getActualImageSize());
    }

    if ( m_time->isHidden() ) {
        if ( m_endDate->date().isValid() )
            info.setDate( DB::ImageDate( QDateTime( m_startDate->date(), QTime(0,0,0) ),
                                     QDateTime( m_endDate->date(), QTime( 23,59,59) ) ) );
        else
            info.setDate( DB::ImageDate( QDateTime( m_startDate->date(), QTime(0,0,0) ),
                                     QDateTime( m_startDate->date(), QTime( 23,59,59) ) ) );
    }
    else
        info.setDate( DB::ImageDate( QDateTime( m_startDate->date(), m_time->time() ) ) );

    // Generate a list of all tagged areas

    QMap<QString, QMap<QString, QRect>> taggedAreas;
    QPair<QString, QString> tagData;

    foreach (ResizableFrame *area, areas())
    {
        tagData = area->tagData();

        if ( !tagData.first.isEmpty() ) {
            taggedAreas[tagData.first][tagData.second] = area->actualCoordinates();
        }
    }

    info.setLabel( m_imageLabel->text() );
    info.setDescription( m_description->toPlainText() );

    Q_FOREACH( ListSelect *ls, m_optionList ) {
        info.setCategoryInfo( ls->category(), ls->itemsOn() );
        if (ls->positionable()) {
            info.setPositionedTags(ls->category(), taggedAreas[ls->category()]);
        }
    }

    if ( m_ratingChanged ) {
        info.setRating( m_rating->rating() );
        m_ratingChanged = false;
    }
}

void AnnotationDialog::Dialog::ShowHideSearch( bool show )
{
    m_megapixel->setVisible( show );
    m_megapixelLabel->setVisible( show );
    m_max_megapixel->setVisible( show );
    m_max_megapixelLabel->setVisible( show );
    m_searchRAW->setVisible( show );
    m_imageFilePatternLabel->setVisible( show );
    m_imageFilePattern->setVisible( show );
    m_isFuzzyDate->setChecked( show );
    m_isFuzzyDate->setVisible( !show );
    slotSetFuzzyDate();
    m_ratingSearchMode->setVisible( show );
    m_ratingSearchLabel->setVisible( show );
}

QList<AnnotationDialog::ResizableFrame *> AnnotationDialog::Dialog::areas() const
{
    return m_preview->preview()->findChildren<ResizableFrame *>();
}


int AnnotationDialog::Dialog::configure( DB::ImageInfoList list, bool oneAtATime )
{
    ShowHideSearch(false);

    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        DB::ImageDB::instance()->categoryCollection()->categoryForName( Settings::SettingsData::instance()->untaggedCategory() )
            ->addItem(Settings::SettingsData::instance()->untaggedTag() );
    }

    if (oneAtATime) {
        m_setup = InputSingleImageConfigMode;
    } else {
        m_setup = InputMultiImageConfigMode;
        // Hide the default positionable category selector
        m_preview->updatePositionableCategories();
    }

#ifdef HAVE_KGEOMAP
    m_mapIsPopulated = false;
    m_annotationMap->clear();
#endif
    m_origList = list;
    m_editList.clear();

    for( DB::ImageInfoListConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        m_editList.append( *(*it) );
    }

    setup();

    if ( oneAtATime )  {
        m_current = 0;
        m_preview->configure( &m_editList, true );
        load();
    }
    else {
        m_preview->configure( &m_editList, false );
        m_preview->canCreateAreas( false );
        m_startDate->setDate( QDate() );
        m_endDate->setDate( QDate() );
        m_time->hide();
        m_rating->setRating( 0 );
        m_ratingChanged = false;
        m_areasChanged = false;

        Q_FOREACH( ListSelect *ls, m_optionList ) {
            setUpCategoryListBoxForMultiImageSelection( ls, list );
        }

        m_imageLabel->setText(QString());
        m_imageFilePattern->setText(QString());
        m_firstDescription = m_editList[0].description();

        const bool allTextEqual =
                std::all_of(m_editList.begin(), m_editList.end(),
                            [=] (const DB::ImageInfo& item) -> bool {
                                   return item.description() == m_firstDescription;
                });

        if ( !allTextEqual )
            m_firstDescription = m_conflictText;
        m_description->setPlainText( m_firstDescription );
    }

    showHelpDialog( oneAtATime ? InputSingleImageConfigMode : InputMultiImageConfigMode );

    return exec();
}

DB::ImageSearchInfo AnnotationDialog::Dialog::search( DB::ImageSearchInfo* search  )
{
    ShowHideSearch(true);

#ifdef HAVE_KGEOMAP
    m_mapIsPopulated = false;
    m_annotationMap->clear();
#endif
    m_setup = SearchMode;
    if ( search )
        m_oldSearch = *search;

    setup();

    m_preview->setImage(Utilities::locateDataFile(QString::fromLatin1("pics/search.jpg")));

    m_ratingChanged = false ;
    showHelpDialog( SearchMode );
    int ok = exec();
    if ( ok == QDialog::Accepted )  {
        const QDateTime start = m_startDate->date().isNull() ? QDateTime() : QDateTime(m_startDate->date());
        const QDateTime end = m_endDate->date().isNull() ? QDateTime() : QDateTime( m_endDate->date() );
        m_oldSearch = DB::ImageSearchInfo( DB::ImageDate( start, end ),
                      m_imageLabel->text(), m_description->toPlainText(),
                      m_imageFilePattern->text());

        Q_FOREACH( const ListSelect *ls, m_optionList ) {
            m_oldSearch.setCategoryMatchText( ls->category(), ls->text() );
        }
        //FIXME: for the user to search for 0-rated images, he must first change the rating to anything > 0
        //then change back to 0 .
        if( m_ratingChanged)
          m_oldSearch.setRating( m_rating->rating() );

        m_ratingChanged = false;
        m_oldSearch.setSearchMode( m_ratingSearchMode->currentIndex() );
        m_oldSearch.setMegaPixel( m_megapixel->value() );
        m_oldSearch.setMaxMegaPixel( m_max_megapixel->value() );
        m_oldSearch.setSearchRAW( m_searchRAW->isChecked() );
#ifdef HAVE_KGEOMAP
        const KGeoMap::GeoCoordinates::Pair regionSelection = m_annotationMap->getRegionSelection();
        m_oldSearch.setRegionSelection(regionSelection);
#endif
        return m_oldSearch;
    }
    else
    return DB::ImageSearchInfo();
}

void AnnotationDialog::Dialog::setup()
{
// Repopulate the listboxes in case data has changed
    // An group might for example have been renamed.
    Q_FOREACH( ListSelect *ls, m_optionList ) {
        ls->populate();
    }

    if ( m_setup == SearchMode )  {
        KGuiItem::assign(m_okBut, KGuiItem(i18nc("@action:button","&Search"), QString::fromLatin1("find")) );
        m_continueLaterBut->hide();
        m_revertBut->hide();
        m_clearBut->show();
        m_preview->setSearchMode(true);
        setWindowTitle( i18nc("@title:window title of the 'find images' window","Search") );
        loadInfo( m_oldSearch );
    }
    else {
        m_okBut->setText( i18n("Done") );
        m_continueLaterBut->show();
        m_revertBut->setEnabled( m_setup == InputSingleImageConfigMode );
        m_clearBut->hide();
        m_revertBut->show();
        m_preview->setSearchMode(false);
        m_preview->setToggleFullscreenPreviewEnabled(m_setup == InputSingleImageConfigMode);
        setWindowTitle( i18nc("@title:window", "Annotations") );
    }

    Q_FOREACH( ListSelect *ls, m_optionList ) {
        ls->setMode( m_setup );
    }
}


void AnnotationDialog::Dialog::slotClear()
{
    loadInfo( DB::ImageSearchInfo() );
}

void AnnotationDialog::Dialog::loadInfo( const DB::ImageSearchInfo& info )
{
    m_startDate->setDate( info.date().start().date() );
    m_endDate->setDate( info.date().end().date() );

    Q_FOREACH( ListSelect *ls, m_optionList ) {
        ls->setText( info.categoryMatchText( ls->category() ) );
    }

    m_imageLabel->setText( info.label() );
    m_description->setText(info.description());
}

void AnnotationDialog::Dialog::slotOptions()
{
    // create menu entries for dock windows
    QMenu* menu = new QMenu( this );
    QMenu* dockMenu =m_dockWindow->createPopupMenu();
    menu->addMenu( dockMenu )
        ->setText( i18n( "Configure Window Layout..." ) );
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
            i18n("Sort by Date"),
            sortTypes );
    alphaTreeSort->setCheckable( true );
    alphaFlatSort->setCheckable( true );
    dateSort->setCheckable( true );
    alphaTreeSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree );
    alphaFlatSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat );
    dateSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse );
    menu->addActions( sortTypes->actions() );
    connect( dateSort, SIGNAL(triggered()), m_optionList.at(0), SLOT(slotSortDate()) );
    connect( alphaTreeSort, SIGNAL(triggered()), m_optionList.at(0), SLOT(slotSortAlphaTree()) );
    connect( alphaFlatSort, SIGNAL(triggered()), m_optionList.at(0), SLOT(slotSortAlphaFlat()) );

    // create MatchType entries
    menu->addSeparator();
    QActionGroup* matchTypes = new QActionGroup( menu );
    QAction* matchFromBeginning = new QAction( i18n( "Match Tags from the First Character"), matchTypes );
    QAction* matchFromWordStart = new QAction( i18n( "Match Tags from Word Boundaries"), matchTypes );
    QAction* matchAnywhere = new QAction( i18n( "Match Tags Anywhere"), matchTypes );
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
    if ( m_setup != SearchMode )
    {
        menu->addSeparator();
        QAction* showSelectedOnly = new QAction(
                SmallIcon( QString::fromLatin1( "view-filter" ) ),
                i18n("Show Only Selected Ctrl+S"),
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
    m_stack->setCurrentWidget( m_dockWindow );
    showTornOfWindows();
    this->setFocus(); // Set temporary focus before show() is called so that extra cursor is not shown on any "random" input widget
    show(); // We need to call show before we call setupFocus() otherwise the widget will not yet all have been moved in place.
    setupFocus();

    const int ret = QDialog::exec();
    hideTornOfWindows();
    return ret;
}

void AnnotationDialog::Dialog::slotSaveWindowSetup()
{
    const QByteArray data = m_dockWindow->saveState();

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
    for( QDockWidget* dock : m_dockWidgets ) {
        if ( dock->isFloating() )
        {
            qCDebug(AnnotationDialogLog) << "Hiding dock: " << dock->objectName();
            dock->hide();
        }
    }
}

void AnnotationDialog::Dialog::showTornOfWindows()
{
    for (QDockWidget* dock: m_dockWidgets ) {
        if ( dock->isFloating() )
        {
            qCDebug(AnnotationDialogLog) << "Showing dock: " << dock->objectName();
            dock->show();
        }
    }
}


AnnotationDialog::ListSelect* AnnotationDialog::Dialog::createListSel( const DB::CategoryPtr& category )
{
    ListSelect* sel = new ListSelect( category, m_dockWindow );
    m_optionList.append( sel );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL(itemRemoved(DB::Category*,QString)),
             this, SLOT(slotDeleteOption(DB::Category*,QString)) );
    connect( DB::ImageDB::instance()->categoryCollection(), SIGNAL(itemRenamed(DB::Category*,QString,QString)),
             this, SLOT(slotRenameOption(DB::Category*,QString,QString)) );

    return sel;
}

void AnnotationDialog::Dialog::slotDeleteOption( DB::Category* category, const QString& value )
{
    for( QList<DB::ImageInfo>::Iterator it = m_editList.begin(); it != m_editList.end(); ++it ) {
        (*it).removeCategoryInfo( category->name(), value );
    }
}

void AnnotationDialog::Dialog::slotRenameOption( DB::Category* category, const QString& oldValue, const QString& newValue )
{
    for( QList<DB::ImageInfo>::Iterator it = m_editList.begin(); it != m_editList.end(); ++it ) {
        (*it).renameItem( category->name(), oldValue, newValue );
    }
}

void AnnotationDialog::Dialog::reject()
{
    if (m_stack->currentWidget() == m_fullScreenPreview) {
        togglePreview();
        return;
    }

    m_fullScreenPreview->stopPlayback();
    if (hasChanges()) {
        int code =  KMessageBox::questionYesNo( this, i18n("<p>Some changes are made to annotations. Do you really want to cancel all recent changes for each affected file?</p>") );
        if ( code == KMessageBox::No )
            return;
    }
    closeDialog();
}

void AnnotationDialog::Dialog::closeDialog()
{
    tidyAreas();
    m_accept = QDialog::Rejected;
    QDialog::reject();
}

bool AnnotationDialog::Dialog::hasChanges()
{
    bool changed = false;
    if ( m_setup == InputSingleImageConfigMode )  {
        writeToInfo();
        for ( int i = 0; i < m_editList.count(); ++i )  {
            changed |= (*(m_origList[i]) != m_editList[i]);
        }
        changed |= m_areasChanged;
    }

    else if ( m_setup == InputMultiImageConfigMode ) {
        changed |= ( !m_startDate->date().isNull() );
        changed |= ( !m_endDate->date().isNull() );

        Q_FOREACH( ListSelect *ls, m_optionList ) {
            StringSet on, partialOn;
            std::tie(on, partialOn) = selectionForMultiSelect( ls, m_origList );
            changed |= (on != ls->itemsOn());
            changed |= (partialOn != ls->itemsUnchanged());
        }

        changed |= ( !m_imageLabel->text().isEmpty() );
        changed |= ( m_description->toPlainText() != m_firstDescription );
        changed |= m_ratingChanged;
    }
    return changed;
}

void AnnotationDialog::Dialog::rotate( int angle )
{
    if ( m_setup == InputMultiImageConfigMode ) {
        // In doneTagging the preview will be queried for its angle.
    }
    else {
        DB::ImageInfo& info = m_editList[ m_current ];
        info.rotate( angle, DB::RotateImageInfoOnly );
        emit imageRotated( info.fileName() );
    }
}

void AnnotationDialog::Dialog::slotSetFuzzyDate()
{
    if ( m_isFuzzyDate->isChecked() )
    {
        m_time->hide();
        m_timeLabel->hide();
        m_endDate->show();
        m_endDateLabel->show();
    } else {
        m_time->show();
        m_timeLabel->show();
        m_endDate->hide();
        m_endDateLabel->hide();
    }
}

void AnnotationDialog::Dialog::slotDeleteImage()
{
    // CTRL+Del is a common key combination when editing text
    // TODO: The word right of cursor should be deleted as expected also in date and category fields
    if ( m_setup == SearchMode )
    return;

    if( m_setup == InputMultiImageConfigMode )  //TODO: probably delete here should mean remove from selection
      return;

    DB::ImageInfoPtr info = m_origList[m_current];

    m_origList.remove( info );
    m_editList.removeAll( m_editList.at( m_current ) );
    MainWindow::DirtyIndicator::markDirty();
    if ( m_origList.count() == 0 ) {
        doneTagging();
        return;
    }
    if ( m_current == (int)m_origList.count() ) // we deleted the last image
        m_current--;

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
        for( QList<QWidget*>::iterator orderedIt = orderedList.begin(); orderedIt != orderedList.end(); ++orderedIt ) {
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
    Q_FOREACH( QWidget *widget, orderedList ) {
        if ( prev ) {
            setTabOrder( prev, widget );
        } else {
            first = widget;
        }
        prev = widget;
    }

    if ( first ) {
        setTabOrder( prev, first );
    }


    // Finally set focus on the first list select
    Q_FOREACH( QWidget *widget, orderedList ) {
        if ( widget->property("FocusCandidate").isValid() && widget->isVisible() ) {
            widget->setFocus();
            break;
        }
    }
}

void AnnotationDialog::Dialog::slotResetLayout()
{
    m_dockWindow->restoreState(m_dockWindowCleanState);
}

void AnnotationDialog::Dialog::slotStartDateChanged( const DB::ImageDate& date )
{
    if ( date.start() == date.end() )
        m_endDate->setDate( QDate() );
    else
        m_endDate->setDate( date.end().date() );
}

void AnnotationDialog::Dialog::loadWindowLayout()
{
    QString fileName =  QString::fromLatin1( "%1/layout.dat" ).arg( Settings::SettingsData::instance()->imageDirectory() );
    if ( !QFileInfo(fileName).exists() )
    {
        // create default layout
        // label/date/rating in a visual block with description:
        m_dockWindow->splitDockWidget(m_generalDock, m_descriptionDock, Qt::Vertical);

        // more space for description:
        m_dockWindow->resizeDocks({m_generalDock, m_descriptionDock},{60,100}, Qt::Vertical);
        // more space for preview:
        m_dockWindow->resizeDocks({m_generalDock, m_descriptionDock, m_previewDock},{200,200,800}, Qt::Horizontal);
#ifdef HAVE_KGEOMAP
        // group the map with the preview
        m_dockWindow->tabifyDockWidget(m_previewDock, m_mapDock);
        // make sure the preview tab is active:
        m_previewDock->raise();
#endif
        return;
    }

    QFile file( fileName );
    file.open( QIODevice::ReadOnly );
    QByteArray data = file.readAll();
    m_dockWindow->restoreState(data);
}

void AnnotationDialog::Dialog::setupActions()
{
    m_actions = new KActionCollection( this );

    QAction * action = nullptr;
    action = m_actions->addAction( QString::fromLatin1("annotationdialog-sort-alphatree"), m_optionList.at(0), SLOT(slotSortAlphaTree()) );
    action->setText( i18n("Sort Alphabetically (Tree)") );
    action->setShortcut(Qt::CTRL+Qt::Key_F4);

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-sort-alphaflat"), m_optionList.at(0), SLOT(slotSortAlphaFlat()) );
    action->setText( i18n("Sort Alphabetically (Flat)") );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-sort-MRU"), m_optionList.at(0), SLOT(slotSortDate()) );
    action->setText( i18n("Sort Most Recently Used") );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-toggle-sort"),  m_optionList.at(0), SLOT(toggleSortType()) );
    action->setText( i18n("Toggle Sorting") );
    action->setShortcut( Qt::CTRL+Qt::Key_T );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-toggle-showing-selected-only"),
                                  &ShowSelectionOnlyManager::instance(), SLOT(toggle()) );
    action->setText( i18n("Toggle Showing Selected Items Only") );
    action->setShortcut( Qt::CTRL+Qt::Key_S );


    action = m_actions->addAction( QString::fromLatin1("annotationdialog-next-image"),  m_preview, SLOT(slotNext()) );
    action->setText(  i18n("Annotate Next") );
    action->setShortcut(  Qt::Key_PageDown );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-prev-image"),  m_preview, SLOT(slotPrev()) );
    action->setText(  i18n("Annotate Previous") );
    action->setShortcut(  Qt::Key_PageUp );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-OK-dialog"),  this, SLOT(doneTagging()) );
    action->setText(  i18n("OK dialog") );
    action->setShortcut(  Qt::CTRL+Qt::Key_Return );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-delete-image"),  this, SLOT(slotDeleteImage()) );
    action->setText(  i18n("Delete") );
    action->setShortcut(  Qt::CTRL+Qt::Key_Delete );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-copy-previous"),  this, SLOT(slotCopyPrevious()) );
    action->setText(  i18n("Copy tags from previous image") );
    action->setShortcut(  Qt::ALT+Qt::Key_Insert );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-rotate-left"),  m_preview, SLOT(rotateLeft()) );
    action->setText(  i18n("Rotate counterclockwise") );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-rotate-right"),  m_preview, SLOT(rotateRight()) );
    action->setText(  i18n("Rotate clockwise") );

    action = m_actions->addAction( QString::fromLatin1("annotationdialog-toggle-viewer"), this, SLOT(togglePreview()) );
    action->setText( i18n("Toggle fullscreen preview") );
    action->setShortcut( Qt::CTRL + Qt::Key_Space );

    foreach (QAction* action, m_actions->actions()) {
      action->setShortcutContext(Qt::WindowShortcut);
      addAction(action);
  }
}

KActionCollection* AnnotationDialog::Dialog::actions()
{
    return m_actions;
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
    m_ratingChanged = true;
}

void AnnotationDialog::Dialog::continueLater()
{
    saveAndClose();
}

void AnnotationDialog::Dialog::saveAndClose()
{
    tidyAreas();

    m_fullScreenPreview->stopPlayback();

    if (m_origList.isEmpty()) {
        // all images are deleted.
        QDialog::accept();
        return;
    }

    // I need to check for the changes first, as the case for m_setup == InputSingleImageConfigMode, saves to the m_origList,
    // and we can thus not check for changes anymore
    const bool anyChanges = hasChanges();

    if ( m_setup == InputSingleImageConfigMode )  {
        writeToInfo();
        for ( int i = 0; i < m_editList.count(); ++i )  {
            *(m_origList[i]) = m_editList[i];
        }
    }
    else if ( m_setup == InputMultiImageConfigMode ) {
        Q_FOREACH( ListSelect *ls, m_optionList ) {
            ls->slotReturn();
        }

        for( DB::ImageInfoListConstIterator it = m_origList.constBegin(); it != m_origList.constEnd(); ++it ) {
            DB::ImageInfoPtr info = *it;
            info->delaySavingChanges(true);
            if ( !m_startDate->date().isNull() )
                info->setDate( DB::ImageDate( m_startDate->date(), m_endDate->date(), m_time->time() ) );

            Q_FOREACH( ListSelect *ls, m_optionList ) {
                info->addCategoryInfo( ls->category(), ls->itemsOn() );
                info->removeCategoryInfo( ls->category(), ls->itemsOff() );
            }

            if ( !m_imageLabel->text().isEmpty() ) {
                info->setLabel( m_imageLabel->text() );
            }


            if ( !m_description->toPlainText().isEmpty() && m_description->toPlainText().compare( m_conflictText ) ) {
                info->setDescription( m_description->toPlainText() );
            }

            if( m_ratingChanged)
            {
              info->setRating( m_rating->rating() );
            }

            info->delaySavingChanges(false);
        }
        m_ratingChanged = false;
    }
    m_accept = QDialog::Accepted;

    if (anyChanges) {
        MainWindow::DirtyIndicator::markDirty();
    }

    QDialog::accept();
}

AnnotationDialog::Dialog::~Dialog()
{
    qDeleteAll( m_optionList );
    m_optionList.clear();
}

void AnnotationDialog::Dialog::togglePreview()
{
    if (m_setup == InputSingleImageConfigMode) {
        if ( m_stack->currentWidget() == m_fullScreenPreview ) {
            m_stack->setCurrentWidget( m_dockWindow );
            m_fullScreenPreview->stopPlayback();
        }
        else {
            m_stack->setCurrentWidget( m_fullScreenPreview );
            m_fullScreenPreview->load( DB::FileNameList() << m_editList[ m_current].fileName() );
        }
    }
}

void AnnotationDialog::Dialog::tidyAreas()
{
    // Remove all areas marked on the preview image
    foreach (ResizableFrame *area, areas()) {
        area->markTidied();
        area->deleteLater();
    }
}

void AnnotationDialog::Dialog::slotNewArea(ResizableFrame *area)
{
    area->setDialog(this);
}

void AnnotationDialog::Dialog::positionableTagSelected(QString category, QString tag)
{
    // Be sure not to propose an already-associated tag
    QPair<QString, QString> tagData = qMakePair(category, tag);
    foreach (ResizableFrame *area, areas()) {
        if (area->tagData() == tagData) {
            return;
        }
    }

    // Set the selected tag as the last selected positionable tag
    m_lastSelectedPositionableTag = tagData;

    // Add the tag to the positionable tag candidate list
    addTagToCandidateList(category, tag);
}

void AnnotationDialog::Dialog::positionableTagDeselected(QString category, QString tag)
{
    // Remove the tag from the candidate list
    removeTagFromCandidateList(category, tag);

    // Search for areas linked against the tag on this image
    if (m_setup == InputSingleImageConfigMode) {
        QPair<QString, QString> deselectedTag = QPair<QString, QString>(category, tag);

        foreach (ResizableFrame *area, areas()) {
            if (area->tagData() == deselectedTag) {
                area->removeTagData();
                m_areasChanged = true;
                // Only one area can be associated with the tag, so we can return here
                return;
            }
        }
    }
    // Removal of tagged areas in InputMultiImageConfigMode is done in DB::ImageInfo::removeCategoryInfo
}

void AnnotationDialog::Dialog::addTagToCandidateList(QString category, QString tag)
{
    m_positionableTagCandidates << QPair<QString, QString>(category, tag);
}

void AnnotationDialog::Dialog::removeTagFromCandidateList(QString category, QString tag)
{
    // Is the deselected tag the last selected positionable tag?
    if (m_lastSelectedPositionableTag.first == category && m_lastSelectedPositionableTag.second == tag) {
        m_lastSelectedPositionableTag = QPair<QString, QString>();
    }

    // Remove the tag from the candidate list
    m_positionableTagCandidates.removeAll(QPair<QString, QString>(category, tag));
    // When a positionable tag is entered via the AreaTagSelectDialog, it's added to this
    // list twice, so we use removeAll here to be sure to also wipe duplicate entries.
}

QPair<QString, QString> AnnotationDialog::Dialog::lastSelectedPositionableTag() const
{
    return m_lastSelectedPositionableTag;
}

QList<QPair<QString, QString>> AnnotationDialog::Dialog::positionableTagCandidates() const
{
    return m_positionableTagCandidates;
}

void AnnotationDialog::Dialog::slotShowAreas(bool showAreas)
{
    foreach (ResizableFrame *area, areas()) {
        area->setVisible(showAreas);
    }
}

void AnnotationDialog::Dialog::positionableTagRenamed(QString category, QString oldTag, QString newTag)
{
    // Is the renamed tag the last selected positionable tag?
    if (m_lastSelectedPositionableTag.first == category && m_lastSelectedPositionableTag.second == oldTag) {
        m_lastSelectedPositionableTag.second = newTag;
    }

    // Check the candidate list for the tag
    QPair<QString, QString> oldTagData = QPair<QString, QString>(category, oldTag);
    if (m_positionableTagCandidates.contains(oldTagData)) {
        // The tag is in the list, so update it
        m_positionableTagCandidates.removeAt(m_positionableTagCandidates.indexOf(oldTagData));
        m_positionableTagCandidates << QPair<QString, QString>(category, newTag);
    }

    // Check if an area on the current image contains the changed or proposed tag
    foreach (ResizableFrame *area, areas()) {
        if (area->tagData() == oldTagData) {
            area->setTagData(category, newTag);
        }

    }
}

void AnnotationDialog::Dialog::descriptionPageUpDownPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp) {
        m_actions->action(QString::fromLatin1("annotationdialog-prev-image"))->trigger();
    } else if (event->key() == Qt::Key_PageDown) {
        m_actions->action(QString::fromLatin1("annotationdialog-next-image"))->trigger();
    }
}

void AnnotationDialog::Dialog::checkProposedTagData(
    QPair<QString, QString> tagData,
    ResizableFrame *areaToExclude) const
{
    foreach (ResizableFrame *area, areas())
    {
        if (area != areaToExclude
            && area->proposedTagData() == tagData
            && area->tagData().first.isEmpty()) {
            area->removeProposedTagData();
        }
    }
}

void AnnotationDialog::Dialog::areaChanged()
{
    m_areasChanged = true;
}

/**
 * @brief positionableTagValid checks whether a given tag can still be associated to an area.
 * This checks for empty and duplicate tags.
 * @return
 */
bool AnnotationDialog::Dialog::positionableTagAvailable(const QString &category, const QString &tag) const
{
    if (category.isEmpty() || tag.isEmpty())
        return false;

    // does any area already have that tag?
    foreach (const ResizableFrame *area, areas())
    {
        const auto tagData = area->tagData();
        if (tagData.first == category && tagData.second == tag)
            return false;
    }

    return true;
}

/**
 * @brief Generates a set of positionable tags currently used on the image
 * @param category
 * @return
 */
QSet<QString> AnnotationDialog::Dialog::positionedTags(const QString &category) const
{
    QSet<QString> tags;
    foreach (const ResizableFrame *area, areas())
    {
        const auto tagData = area->tagData();
        if (tagData.first == category)
            tags += tagData.second;
    }
    return tags;
}

AnnotationDialog::ListSelect *AnnotationDialog::Dialog::listSelectForCategory(const QString &category)
{
    return m_listSelectList.value(category,nullptr);
}

#ifdef HAVE_KGEOMAP
void AnnotationDialog::Dialog::updateMapForCurrentImage()
{
    if (m_setup != InputSingleImageConfigMode) {
        return;
    }

    if (m_editList[m_current].coordinates().hasCoordinates()) {
        m_annotationMap->setCenter(m_editList[m_current]);
        m_annotationMap->displayStatus(Map::MapView::MapStatus::ImageHasCoordinates);
    } else {
        m_annotationMap->displayStatus(Map::MapView::MapStatus::ImageHasNoCoordinates);
    }
}

void AnnotationDialog::Dialog::annotationMapVisibilityChanged(bool visible)
{
    // This populates the map if it's added when the dialog is already open
    if ( visible ) {
        // when the map dockwidget is already visible on show(), the call to
        // annotationMapVisibilityChanged  is executed in the GUI thread.
        // This ensures that populateMap() doesn't block the GUI in this case:
        QTimer::singleShot(0, this, SLOT(populateMap()));
    } else {
        m_cancelMapLoading = true;
    }
}

void AnnotationDialog::Dialog::populateMap()
{
    // populateMap is called every time the map widget gets visible
    if (m_mapIsPopulated) {
        return;
    }
    m_annotationMap->displayStatus(Map::MapView::MapStatus::Loading);
    m_cancelMapLoading = false;
    m_mapLoadingProgress->setMaximum(m_editList.count());
    m_mapLoadingProgress->show();
    m_cancelMapLoadingButton->show();

    int processedImages = 0;
    int imagesWithCoordinates = 0;

    foreach (DB::ImageInfo info, m_editList) {
        processedImages++;
        m_mapLoadingProgress->setValue(processedImages);
        // keep things responsive by processing events manually:
        QApplication::processEvents();

        if (info.coordinates().hasCoordinates()) {
            m_annotationMap->addImage(info);
            imagesWithCoordinates++;
        }

        // m_cancelMapLoading is set to true by clicking the "Cancel" button
        if (m_cancelMapLoading) {
            m_annotationMap->clear();
            break;
        }
    }

    // at this point either we canceled loading or the map is populated:
    m_mapIsPopulated = ! m_cancelMapLoading;
    mapLoadingFinished(imagesWithCoordinates > 0, imagesWithCoordinates == processedImages);
}

void AnnotationDialog::Dialog::setCancelMapLoading()
{
    m_cancelMapLoading = true;
}

void AnnotationDialog::Dialog::mapLoadingFinished(bool mapHasImages, bool allImagesHaveCoordinates)
{
    m_mapLoadingProgress->hide();
    m_cancelMapLoadingButton->hide();

    if (m_setup == InputSingleImageConfigMode) {
        m_annotationMap->displayStatus(Map::MapView::MapStatus::ImageHasNoCoordinates);
    } else {
        if (m_setup == SearchMode) {
            m_annotationMap->displayStatus(Map::MapView::MapStatus::SearchCoordinates);
        } else {
            if (mapHasImages) {
                if (! allImagesHaveCoordinates) {
                    m_annotationMap->displayStatus(Map::MapView::MapStatus::SomeImagesHaveNoCoordinates);
                } else {
                    m_annotationMap->displayStatus(Map::MapView::MapStatus::ImageHasCoordinates);
                }
            } else {
                m_annotationMap->displayStatus(Map::MapView::MapStatus::NoImagesHaveNoCoordinates);
            }
        }
    }

    if (m_setup != SearchMode) {
        m_annotationMap->zoomToMarkers();
        updateMapForCurrentImage();
    }
}
#endif

// vi:expandtab:tabstop=4 shiftwidth=4:
