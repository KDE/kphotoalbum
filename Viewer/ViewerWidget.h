/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#ifndef VIEWER_H
#define VIEWER_H

#include <DB/FileNameList.h>
#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>

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
class AbstractDisplay;
class ImageDisplay;
class InfoBox;
class SpeedDisplay;
class TextDisplay;
class VideoDisplay;
class VideoShooter;

class ViewerWidget : public QStackedWidget
{
    Q_OBJECT
public:
    enum UsageType { InlineViewer,
                     ViewerWindow };

    ViewerWidget(UsageType type = ViewerWindow,
                 QMap<Qt::Key, QPair<QString, QString>> *macroStore = nullptr);
    ~ViewerWidget() override;
    static ViewerWidget *latest();
    void load(const DB::FileNameList &list, int index = 0);
    void infoBoxMove();
    bool showingFullScreen() const;
    void setShowFullScreen(bool on);
    void show(bool slideShow);
    KActionCollection *actions();

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

public slots:
    void updateInfoBox();
    void test();
    void moveInfoBox(int);
    void stopPlayback();
    void remapAreas(QSize viewSize, QRect zoomWindow, double sizeRatio);
    void copyTo();

signals:
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
    void inhibitScreenSaver(bool inhibit);
    DB::ImageInfoPtr currentInfo() const;
    friend class InfoBox;

    void updatePalette();

private:
    void showNextN(int);
    void showPrevN(int);
    int find_tag_in_list(const QStringList &list, QString &namefound);
    void invalidateThumbnail() const;
    enum RemoveAction { RemoveImageFromDatabase,
                        OnlyRemoveFromViewer };
    void removeOrDeleteCurrent(RemoveAction);

protected slots:
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
    void rotate90();
    void rotate180();
    void rotate270();
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

    /** Set the current window title (filename) and add the given detail */
    void setCaptionWithDetail(const QString &detail);

    /**
     * @brief slotRemoveDeletedImages removes all deleted images from the viewer playback list.
     * @param imageList
     */
    void slotRemoveDeletedImages(const DB::FileNameList &imageList);

private:
    static ViewerWidget *s_latest;
    friend class VideoShooter;

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

    QAction *m_copyTo;

    InfoBox *m_infoBox;
    QImage m_currentImage;

    bool m_showingFullScreen;

    int m_slideShowPause;
    SpeedDisplay *m_speedDisplay;
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

    enum InputMode { InACategory,
                     AlwaysStartWithCategory };

    InputMode m_currentInputMode;
    QString m_currentInput;
    QString m_currentCategory;
    QString m_currentInputList;

    QString m_lastFound;
    QString m_lastCategory;
    QMap<Qt::Key, QPair<QString, QString>> *m_inputMacros;
    QMap<Qt::Key, QPair<QString, QString>> *m_myInputMacros;

    QString m_lastCopyToTarget;
};

}

#endif /* VIEWER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
