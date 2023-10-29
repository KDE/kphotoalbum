// SPDX-FileCopyrightText: 2003-2023 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIEWER_H
#define VIEWER_H

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <MainWindow/CopyLinkEngine.h>
#include <kpabase/CrashSentinel.h>
#include <kpabase/FileNameList.h>

#include <QImage>
#include <QMap>
#include <QPixmap>
#include <QPointer>
#include <QStackedWidget>

class KActionCollection;
class QAction;
class QContextMenuEvent;
class QKeyEvent;
class QMenu;
class QResizeEvent;
class QStackedWidget;
class QWheelEvent;
class CursorVisibilityHandler;
class QLabel;

namespace DB
{
class ImageInfo;
class Id;
}
namespace MainWindow
{
class ExternalPopup;
class CategoryImagePopup;
}
namespace Exif
{
class InfoDialog;
}
namespace Viewer
{

class AnnotationHandler;
class AbstractDisplay;
class ImageDisplay;
class InfoBox;
class TransientDisplay;
class TextDisplay;
class VideoDisplay;
class VideoShooter;

class ViewerWidget : public QStackedWidget
{
    Q_OBJECT
public:
    enum class UsageType { CompactPreview, //< Preview ViewerWidget with no (internal) controls. As used in the annotation dialog preview dock.
                           FullsizePreview, ///< Preview ViewerWidget with minimal context menu that can be used for a full size window. As used in the "fullscreen" preview of tha annotation dialog.
                           FullFeaturedViewer ///< Full featured ViewerWidget.
    };

    ViewerWidget(UsageType type = UsageType::FullFeaturedViewer);
    ~ViewerWidget() override;
    static ViewerWidget *latest();
    void load(const DB::FileNameList &list, int index = 0);
    void infoBoxMove();
    bool showingFullScreen() const;
    void setShowFullScreen(bool on);
    void show(bool slideShow);
    KActionCollection *actions();
    void setCopyLinkEngine(MainWindow::CopyLinkEngine *copyLinkEngine);

    /**
     * @brief setTaggedAreasFromImage
     * Clear existing areas and set them based on the currentInfo().
     */
    void setTaggedAreasFromImage();
    /**
     * @brief addAdditionalTaggedAreas adds additional areas and marks them as highlighted.
     * @param taggedAreas
     */
    void addAdditionalTaggedAreas(DB::TaggedAreas taggedAreas);

public Q_SLOTS:
    void updateInfoBox();
    void test();
    void moveInfoBox(int);
    void stopPlayback();
    void remapAreas(QSize viewSize, QRect zoomWindow, double sizeRatio);

Q_SIGNALS:
    void soughtTo(const DB::FileName &id);
    void imageRotated(const DB::FileName &id);

protected:
    void closeEvent(QCloseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void wheelEvent(QWheelEvent *event) override;

    void moveInfoBox();

    enum class AreaType { Standard,
                          Highlighted };
    /**
     * @brief addTaggedAreas adds tagged areas to the viewer.
     * @param taggedAreas Map(category -> Map(tagname, area))
     * @param type AreaType::Standard is for areas that are part of the Image; AreaType::Highlight is for additional areas
     */
    void addTaggedAreas(DB::TaggedAreas taggedAreas, AreaType type);
    void load();
    void setupContextMenu();
    void createShowContextMenu();
    void createInvokeExternalMenu();
    void createRotateMenu();
    void createSkipMenu();
    void createZoomMenu();
    void createSlideShowMenu();
    void createVideoMenu();
    void createCategoryImageMenu();
    void createFilterMenu();
    void changeSlideShowInterval(int delta);
    void createVideoViewer();
    void createAnnotationMenu();
    void inhibitScreenSaver(bool inhibit);
    /**
     * @brief currentFileName accesses the current file name in a safe way.
     * If the current index is invalid, a \c null DB::FileName is returned.
     * @return the current DB::FileName
     */
    DB::FileName currentFileName() const;
    /**
     * @brief currentInfo queries the database for the ImageInfo for the current file.
     * If the current index is invalid, a \c null DB::ImageInfoPtr is returned.
     * @return the ImageInfoPtr for the current DB::FileName.
     */
    DB::ImageInfoPtr currentInfo() const;
    friend class InfoBox;

    void updatePalette();

private:
    void showNextN(int);
    void showPrevN(int);
    void invalidateThumbnail() const;
    enum RemoveAction { RemoveImageFromDatabase,
                        OnlyRemoveFromViewer };
    void removeOrDeleteCurrent(RemoveAction);

    enum class TagMode { Locked,
                         Annotating,
                         Tokenizing };
    void setTagMode(TagMode tagMode);
    void updateContextMenuState(bool isVideo);

protected Q_SLOTS:
    void showNext();
    void showNext10();
    void showNext100();
    void showNext1000();
    void showPrev();
    void showPrev10();
    void showPrev100();
    void showPrev1000();
    void showFirst();
    void showLast();
    void deleteCurrent();
    void removeCurrent();
    void rotate(int angle);
    void toggleFullScreen();
    void slotStartStopSlideShow();
    void slotSlideShowNext();
    void slotSlideShowNextFromTimer();
    void slotSlideShowFaster();
    void slotSlideShowSlower();
    void editImage();
    void filterNone();
    void filterSelected();
    void filterBW();
    void filterContrastStretch();
    void filterHistogramEqualization();
    void filterMono();
    void slotSetStackHead();
    void updateCategoryConfig();
    void populateExternalPopup();
    void populateCategoryImagePopup();
    void videoStopped();
    void showExifViewer();
    void zoomIn();
    void zoomOut();
    void zoomFull();
    void zoomPixelForPixel();
    void makeThumbnailImage();
    void addTag();
    void editDescription();
    void showAnnotationHelp();

    /** Set the current window title (filename) and add the given detail */
    void setCaptionWithDetail(const QString &detail);

    /**
     * @brief slotRemoveDeletedImages removes all deleted images from the viewer playback list.
     * @param imageList
     */
    void slotRemoveDeletedImages(const DB::FileNameList &imageList);

    void triggerCopyLinkAction(MainWindow::CopyLinkEngine::Action action);
    void toggleTag(const QString &category, const QString &value);
    void copyTagsFromPreviousImage();

private:
    static ViewerWidget *s_latest;
    friend class VideoShooter;
    friend class TemporarilyDisableCursorHandling;

    QList<QAction *> m_forwardActions;
    QList<QAction *> m_backwardActions;

    QAction *m_startStopSlideShow;
    QAction *m_slideShowRunFaster;
    QAction *m_slideShowRunSlower;
    QAction *m_setStackHead;
    QAction *m_filterNone;
    QAction *m_filterSelected;
    QAction *m_filterBW;
    QAction *m_filterContrastStretch;
    QAction *m_filterHistogramEqualization;
    QAction *m_filterMono;

    AbstractDisplay *m_display;
    ImageDisplay *m_imageDisplay;
    VideoDisplay *m_videoDisplay;
    TextDisplay *m_textDisplay;

    KPABase::CrashSentinel m_crashSentinel;

    int m_screenSaverCookie;
    DB::FileNameList m_list;
    DB::FileNameList m_removed;
    int m_current;
    QRect m_textRect;
    QMenu *m_popup;
    QMenu *m_rotateMenu;
    QMenu *m_filterMenu;
    MainWindow::ExternalPopup *m_externalPopup;
    MainWindow::CategoryImagePopup *m_categoryImagePopup;
    int m_width;
    int m_height;
    QPixmap m_pixmap;

    QAction *m_delete;
    QAction *m_showExifViewer;
    QPointer<Exif::InfoDialog> m_exifViewer;

    QAction *m_copyToAction;
    QAction *m_linkToAction;

    InfoBox *m_infoBox;

    bool m_showingFullScreen;

    int m_slideShowPause;
    TransientDisplay *m_transientDisplay;
    KActionCollection *m_actions;
    bool m_forward;
    QTimer *m_slideShowTimer;
    bool m_isRunningSlideShow;

    QList<QAction *> m_videoActions;
    QAction *m_stop;
    QAction *m_playPause;
    QAction *m_makeThumbnailImage;
    bool m_videoPlayerStoppedManually;
    UsageType m_type;

    MainWindow::CopyLinkEngine *m_copyLinkEngine;
    CursorVisibilityHandler *m_cursorHandlerForImageDisplay;
    CursorVisibilityHandler *m_cursorHandlerForVideoDisplay;
    AnnotationHandler *m_annotationHandler;

    TagMode m_tagMode = TagMode::Locked;
    QAction *m_addTagAction;
    QAction *m_copyAction;
    QAction *m_addDescriptionAction;
};

}

#endif /* VIEWER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
