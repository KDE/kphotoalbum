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
#include <kdialog.h>

class QStackedWidget;
class KActionCollection;
class QMoveEvent;
class QResizeEvent;
class QCloseEvent;
class KComboBox;
class KTextEdit;
class DockWidget;
class QDockWidget;
class QTimeEdit;
class QMainWindow;
class QSplitter;
class KPushButton;
class KLineEdit;
class KPushButton;

class KRatingWidget;

namespace Viewer
{
    class ViewerWidget;
}

namespace DB
{
    class ImageInfo;
}


namespace AnnotationDialog
{
class ImagePreview;
class KDateEdit;
class ShortCutManager;
class ResizableFrame;

class Dialog :public KDialog {
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
    QString localizedCategory(QString category) const;

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
    QStackedWidget* _stack;
    Viewer::ViewerWidget* _fullScreenPreview;
    DB::ImageInfoList _origList;
    QList<DB::ImageInfo> _editList;
    int _current;
    UsageMode _setup;
    QList< ListSelect* > _optionList;
    DB::ImageSearchInfo _oldSearch;
    QSplitter* _splitter;
    int _accept;
    QList<QDockWidget*> _dockWidgets;

    // Widgets
    QMainWindow* _dockWindow;
    KLineEdit* _imageLabel;
    KDateEdit* _startDate;
    KDateEdit* _endDate;
    QLabel* _endDateLabel;
    QLabel* _imageFilePatternLabel;
    KLineEdit* _imageFilePattern;

    ImagePreviewWidget* _preview;
    KPushButton* _revertBut;
    KPushButton* _clearBut;
    KPushButton* _okBut;
    KPushButton* _continueLaterBut;
    KTextEdit* _description;
    QTimeEdit* _time;
    QLabel* _timeLabel;
    QCheckBox* _isFuzzyDate;
    KRatingWidget* _rating;
    KComboBox* _ratingSearchMode;
    QLabel* _ratingSearchLabel;
    bool _ratingChanged;
    QSpinBox* _megapixel;
    QLabel* _megapixelLabel;
    QCheckBox* _searchRAW;
    QString conflictText;
    QString _firstDescription;

    KActionCollection* _actions;

    /** Clean state of the dock window.
     *
     * Used in slotResetLayout().
     */
    QByteArray _dockWindowCleanState;
    void tidyAreas();
    QPair<QString, QString> _lastSelectedPositionableTag;
    QList<QPair<QString, QString>> _positionableTagCandidates;
    QMap<QString, ListSelect*> _listSelectList;
    QMap<QString, QString> _categoryL10n;

    bool _positionableCategories;
};

}

#endif /* ANNOTATIONDIALOG_DIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
