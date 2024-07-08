// SPDX-FileCopyrightText: 2003-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005 Stephan Binner <binner@kde.org>
// SPDX-FileCopyrightText: 2005-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007-2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2009 Hassan Ibraheem <hasan.ibraheem@gmail.com>
// SPDX-FileCopyrightText: 2011-2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2012-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
// SPDX-FileCopyrightText: 2014-2024 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2017-2020 Robert Krawitz <rlk@alum.mit.edu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ANNOTATIONDIALOG_DIALOG_H
#define ANNOTATIONDIALOG_DIALOG_H

#include <kpabase/config-kpa-marble.h>

#include "ImagePreviewWidget.h"
#include "ListSelect.h"

#include <DB/Category.h>
#include <DB/ImageInfoList.h>
#include <DB/search/ImageSearchInfo.h>
#include <kpabase/StringSet.h>
#include <kpabase/enums.h>

#include <QCheckBox>
#include <QDialog>
#include <QList>
#include <QSpinBox>

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
class DescriptionEdit;
class ShortCutManager;
class ResizableFrame;

class Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent);
    ~Dialog() override;
    int configure(DB::ImageInfoList list, bool oneAtATime);
    DB::ImageSearchInfo search(DB::ImageSearchInfo *search = nullptr);
    KActionCollection *actions();
    QPair<QString, QString> lastSelectedPositionableTag() const;
    QList<QPair<QString, QString>> positionableTagCandidates() const;
    void addTagToCandidateList(QString category, QString tag);
    void removeTagFromCandidateList(QString category, QString tag);
    void checkProposedTagData(QPair<QString, QString> tagData, ResizableFrame *areaToExclude) const;
    void areaChanged();
    bool positionableTagAvailable(const QString &category, const QString &tag) const;
    QSet<QString> positionedTags(const QString &category) const;
    /**
     * @return A list of all ResizableFrame objects on the current image
     */
    QList<ResizableFrame *> areas() const;
    /**
     * @brief taggedAreas creates a map of all the currently tagged areas.
     * This is different from areas(), which also contains untagged areas.
     * This is different from \code m_editList[m_current].areas()\endcode, which
     * does not include newly added (or deleted) areas.
     * @return a map of currently tagged areas
     */
    DB::TaggedAreas taggedAreas() const;
    ListSelect *listSelectForCategory(const QString &category);

protected Q_SLOTS:
    void slotRevert();
    void slotIndexChanged(int index);
    void doneTagging();
    void continueLater();
    void slotClearSearchForm();
    void slotOptions();
    void slotSaveWindowSetup();
    void slotDeleteOption(DB::Category *, const QString &);
    void slotRenameOption(DB::Category *, const QString &, const QString &);
    void reject() override;
    void rotate(int angle);
    void slotSetFuzzyDate(bool checked);
    void slotResetLayout();
    void slotStartDateChanged(const DB::ImageDate &);
    void slotCopyPrevious();
    void slotShowAreas(bool showAreas);
    void slotRatingChanged(int);
    void togglePreview();
    void descriptionPageUpDownPressed(QKeyEvent *event);
    void slotNewArea(AnnotationDialog::ResizableFrame *area);
    void positionableTagSelected(QString category, QString tag);
    void positionableTagDeselected(QString category, QString tag);
    void positionableTagRenamed(QString category, QString oldTag, QString newTag);
#ifdef HAVE_MARBLE
    void setCancelMapLoading();
    void annotationMapVisibilityChanged(bool visible);
    void populateMap();
#endif
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    /**
     * @brief slotDiscardFiles removes a list of files from the annotation dialog as if they were discarded.
     * The main usage is to remove files from the dialog if they are deleted, avoiding stale image data from showing up.
     *
     * Files that are not currently in the list of annotated files are silently ignored.
     * @param files
     */
    void slotDiscardFiles(const DB::FileNameList &files);

Q_SIGNALS:
    void imageRotated(const DB::FileName &id);

protected:
    QDockWidget *createDock(const QString &title, const QString &name, Qt::DockWidgetArea location, QWidget *widget);
    QWidget *createDateWidget(ShortCutManager &shortCutManager);
    ListSelect *createListSel(const DB::CategoryPtr &category);

    void load();
    void writeToInfo();
    void setup();
    void loadInfo(const DB::ImageSearchInfo &);
    int exec() override;
    void closeEvent(QCloseEvent *) override;
    void showFloatingWindows();
    void hideFloatingWindows();
    bool hasChanges();
    StringSet changedOptions(const ListSelect *);
    void showHelpDialog(UsageMode);
    void resizeEvent(QResizeEvent *) override;
    void moveEvent(QMoveEvent *) override;
    void setupFocus();
    void closeDialog();
    void loadWindowLayout();
    void setupActions();
    void setUpCategoryListBoxForMultiImageSelection(ListSelect *, const DB::ImageInfoList &images);
    std::tuple<Utilities::StringSet, Utilities::StringSet, Utilities::StringSet> selectionForMultiSelect(const ListSelect *, const DB::ImageInfoList &images);
    void saveAndClose();
    void ShowHideSearch(bool show);
#ifdef HAVE_MARBLE
    void clearMapData();
#endif

private:
    QStackedWidget *m_stack;
    Viewer::ViewerWidget *m_fullScreenPreview;
    DB::ImageInfoList m_origList;
    QList<DB::ImageInfo> m_editList;
    int m_current;
    UsageMode m_setup;
    QList<ListSelect *> m_optionList;
    DB::ImageSearchInfo m_oldSearch;
    int m_accept;
    QList<QDockWidget *> m_dockWidgets;
    // "special" named dockWidgets (used to set default layout):
    QDockWidget *m_generalDock;
    QDockWidget *m_previewDock;
    QDockWidget *m_descriptionDock;

    // Widgets
    QMainWindow *m_dockWindow;
    KLineEdit *m_imageLabel;
    DateEdit *m_startDate;
    DateEdit *m_endDate;
    QLabel *m_endDateLabel;
    QLabel *m_imageFilePatternLabel;
    KLineEdit *m_imageFilePattern;

    ImagePreviewWidget *m_preview;
    QPushButton *m_revertBut;
    QPushButton *m_clearBut;
    QPushButton *m_okBut;
    QPushButton *m_continueLaterBut;
    DescriptionEdit *m_description;
    QTimeEdit *m_time;
    QLabel *m_timeLabel;
    QCheckBox *m_isFuzzyDate;
    KRatingWidget *m_rating;
    KComboBox *m_ratingSearchMode;
    QLabel *m_ratingSearchLabel;
    bool m_ratingChanged;
    QSpinBox *m_megapixel;
    QLabel *m_megapixelLabel;
    QSpinBox *m_max_megapixel;
    QLabel *m_max_megapixelLabel;
    QCheckBox *m_searchRAW;
    QString m_conflictText;
    QString m_firstDescription;

    KActionCollection *m_actions;

    /** Clean state of the dock window.
     *
     * Used in slotResetLayout().
     */
    QByteArray m_dockWindowCleanState;
    void tidyAreas();
    QPair<QString, QString> m_lastSelectedPositionableTag;
    QList<QPair<QString, QString>> m_positionableTagCandidates;
    QMap<QString, ListSelect *> m_listSelectList;

    bool m_positionableCategories;
    bool m_areasChanged;

#ifdef HAVE_MARBLE
    QDockWidget *m_mapDock;
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
