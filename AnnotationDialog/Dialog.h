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

#ifndef ANNOTATIONDIALOG_DIALOG_H
#define ANNOTATIONDIALOG_DIALOG_H

#include "Utilities/Set.h"
#include "ListSelect.h"
#include "DB/ImageSearchInfo.h"
#include <QList>
#include <QSpinBox>
#include "DB/ImageInfoList.h"
#include "DB/Category.h"
#include "enums.h"
#include "ImagePreviewWidget.h"
#include <QCheckBox>
#include <QDialog>
#include "config-kpa-kgeomap.h"

class DockWidget;
class KActionCollection;
class KComboBox;
class KLineEdit;
class KRatingWidget;
class KTextEdit;
class QCloseEvent;
class QDockWidget;
class QMainWindow;
class QMoveEvent;
class QProgressBar;
class QPushButton;
class QResizeEvent;
class QSplitter;
class QStackedWidget;
class QTimeEdit;

namespace Viewer
{
    class ViewerWidget;
}

namespace DB
{
    class ImageInfo;
}

namespace Map
{
    class MapView;
}

namespace AnnotationDialog
{
class ImagePreview;
class DateEdit;
class ShortCutManager;
class ResizableFrame;

class Dialog :public QDialog {
    Q_OBJECT
public:
    explicit Dialog( QWidget* parent );
    ~Dialog();
    int configure( DB::ImageInfoList list,  bool oneAtATime );
    DB::ImageSearchInfo search( DB::ImageSearchInfo* search = nullptr );
    KActionCollection* actions();
    QPair<QString, QString> lastSelectedPositionableTag() const;
    QList<QPair<QString, QString>> positionableTagCandidates() const;
    void addTagToCandidateList(QString category, QString tag);
    void removeTagFromCandidateList(QString category, QString tag);
    void checkProposedTagData(QPair<QString, QString> tagData, ResizableFrame *areaToExclude) const;
    void areaChanged();

protected slots:
    void slotRevert();
    void slotIndexChanged( int index );
    void doneTagging();
    void continueLater();
    void slotClear();
    void slotOptions();
    void slotSaveWindowSetup();
    void slotDeleteOption( DB::Category*, const QString& );
    void slotRenameOption( DB::Category* , const QString& , const QString&  );
    virtual void reject();
    void rotate( int angle );
    void slotSetFuzzyDate();
    void slotDeleteImage();
    void slotResetLayout();
    void slotStartDateChanged( const DB::ImageDate& );
    void slotCopyPrevious();
    void slotShowAreas(bool showAreas);
    void slotRatingChanged( unsigned int );
    void togglePreview();
    void descriptionPageUpDownPressed(QKeyEvent *event);
    void slotNewArea(ResizableFrame *area);
    void positionableTagSelected(QString category, QString tag);
    void positionableTagDeselected(QString category, QString tag);
    void positionableTagRenamed(QString category, QString oldTag, QString newTag);
#ifdef HAVE_KGEOMAP
    void setCancelMapLoading();
    void annotationMapVisibilityChanged(bool visible);
    void populateMap();
#endif

signals:
    void imageRotated(const DB::FileName& id);

protected:
    QDockWidget* createDock( const QString& title, const QString& name, Qt::DockWidgetArea location, QWidget* widget );
    QWidget* createDateWidget(ShortCutManager& shortCutManager);
    QWidget* createPreviewWidget();
    ListSelect* createListSel( const DB::CategoryPtr& category );

    void load();
    void writeToInfo();
    void setup();
    void loadInfo( const DB::ImageSearchInfo& );
    int exec();
    virtual void closeEvent( QCloseEvent* );
    void showTornOfWindows();
    void hideTornOfWindows();
    bool hasChanges();
    void showHelpDialog( UsageMode );
    virtual void resizeEvent( QResizeEvent* );
    virtual void moveEvent ( QMoveEvent * );
    void setupFocus();
    void closeDialog();
    void loadWindowLayout();
    void setupActions();
    void setUpCategoryListBoxForMultiImageSelection( ListSelect*, const DB::ImageInfoList& images );
    std::tuple<Utilities::StringSet, Utilities::StringSet> selectionForMultiSelect( ListSelect*, const DB::ImageInfoList& images );
    void saveAndClose();
    void ShowHideSearch( bool show );

private:
    QStackedWidget* m_stack;
    Viewer::ViewerWidget* m_fullScreenPreview;
    DB::ImageInfoList m_origList;
    QList<DB::ImageInfo> m_editList;
    int m_current;
    UsageMode m_setup;
    QList< ListSelect* > m_optionList;
    DB::ImageSearchInfo m_oldSearch;
    int m_accept;
    QList<QDockWidget*> m_dockWidgets;

    // Widgets
    QMainWindow* m_dockWindow;
    KLineEdit* m_imageLabel;
    DateEdit* m_startDate;
    DateEdit* m_endDate;
    QLabel* m_endDateLabel;
    QLabel* m_imageFilePatternLabel;
    KLineEdit* m_imageFilePattern;

    ImagePreviewWidget* m_preview;
    QPushButton* m_revertBut;
    QPushButton* m_clearBut;
    QPushButton* m_okBut;
    QPushButton* m_continueLaterBut;
    KTextEdit* m_description;
    QTimeEdit* m_time;
    QLabel* m_timeLabel;
    QCheckBox* m_isFuzzyDate;
    KRatingWidget* m_rating;
    KComboBox* m_ratingSearchMode;
    QLabel* m_ratingSearchLabel;
    bool m_ratingChanged;
    QSpinBox* m_megapixel;
    QLabel* m_megapixelLabel;
    QCheckBox* m_searchRAW;
    QString m_conflictText;
    QString m_firstDescription;

    KActionCollection* m_actions;

    /** Clean state of the dock window.
     *
     * Used in slotResetLayout().
     */
    QByteArray m_dockWindowCleanState;
    void tidyAreas();
    QPair<QString, QString> m_lastSelectedPositionableTag;
    QList<QPair<QString, QString>> m_positionableTagCandidates;
    QMap<QString, ListSelect*> m_listSelectList;

    bool m_positionableCategories;
    bool m_areasChanged;

#ifdef HAVE_KGEOMAP
    QWidget *m_annotationMapContainer;
    Map::MapView *m_annotationMap;
    void updateMapForCurrentImage();
    QProgressBar *m_mapLoadingProgress;
    QPushButton *m_cancelMapLoadingButton;
    void mapLoadingFinished(bool mapHasImages, bool allImagesHaveCoordinates);
    bool m_cancelMapLoading;
    bool m_mapIsPopulated;
#endif
};

}

#endif /* ANNOTATIONDIALOG_DIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
