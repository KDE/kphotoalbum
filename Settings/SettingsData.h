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

#ifndef SETTINGS_SETTINGSDATA_H
#define SETTINGS_SETTINGSDATA_H

#include <AnnotationDialog/enums.h>
#include <DB/ImageSearchInfo.h>
#include <Utilities/StringSet.h>
#include <QObject>

#define property(GET_TYPE, GET_FUNC, SET_FUNC, SET_TYPE) \
    GET_TYPE GET_FUNC() const;                           \
    void SET_FUNC(const SET_TYPE)

#define property_copy(GET_FUNC, SET_FUNC, TYPE) property(TYPE, GET_FUNC, SET_FUNC, TYPE)
#define property_ref(GET_FUNC, SET_FUNC, TYPE) property(TYPE, GET_FUNC, SET_FUNC, TYPE &)

namespace DB
{
class CategoryCollection;
}

namespace Settings
{
using Utilities::StringSet;

enum Position { Bottom,
                Top,
                Left,
                Right,
                TopLeft,
                TopRight,
                BottomLeft,
                BottomRight };
enum ViewSortType { SortLastUse,
                    SortAlphaTree,
                    SortAlphaFlat };
enum TimeStampTrust { Always,
                      Ask,
                      Never };
enum StandardViewSize { FullSize,
                        NaturalSize,
                        NaturalSizeIfFits };
enum ThumbnailAspectRatio { Aspect_1_1,
                            Aspect_4_3,
                            Aspect_3_2,
                            Aspect_16_9,
                            Aspect_3_4,
                            Aspect_2_3,
                            Aspect_9_16 };
enum LoadOptimizationPreset { LoadOptimizationHardDisk,
                              LoadOptimizationNetwork,
                              LoadOptimizationSataSSD,
                              LoadOptimizationSlowNVME,
                              LoadOptimizationFastNVME,
                              LoadOptimizationManual };

typedef const char *WindowType;
extern const WindowType MainWindow, AnnotationDialog;

class SettingsData : public QObject
{
    Q_OBJECT

public:
    static SettingsData *instance();
    static bool ready();
    static void setup(const QString &imageDirectory);

    /////////////////
    //// General ////
    /////////////////

    property_ref(histogramSize, setHistogramSize, QSize);
    property_copy(useEXIFRotate, setUseEXIFRotate, bool);
    property_copy(useEXIFComments, setUseEXIFComments, bool);
    property_copy(stripEXIFComments, setStripEXIFComments, bool);
    property_copy(commentsToStrip, setCommentsToStrip, QString);
    property_copy(searchForImagesOnStart, setSearchForImagesOnStart, bool);
    property_copy(ignoreFileExtension, setIgnoreFileExtension, bool);
    property_copy(skipSymlinks, setSkipSymlinks, bool);
    property_copy(skipRawIfOtherMatches, setSkipRawIfOtherMatches, bool);
    property_copy(useRawThumbnail, setUseRawThumbnail, bool);
    property_copy(useRawThumbnailSize, setUseRawThumbnailSize, QSize);
    property_copy(useCompressedIndexXML, setUseCompressedIndexXML, bool);
    property_copy(compressBackup, setCompressBackup, bool);
    property_copy(showSplashScreen, setShowSplashScreen, bool);
    property_copy(showHistogram, setShowHistogram, bool);
    property_copy(histogramUseLinearScale, setHistogramUseLinearScale, bool);
    property_copy(autoSave, setAutoSave, int);
    property_copy(backupCount, setBackupCount, int);
    property_copy(viewSortType, setViewSortType, ViewSortType);
    property_copy(matchType, setMatchType, AnnotationDialog::MatchType);
    property_copy(tTimeStamps, setTTimeStamps, TimeStampTrust);
    property_copy(excludeDirectories, setExcludeDirectories, QString);
#ifdef KPA_ENABLE_REMOTECONTROL
    property_copy(recentAndroidAddress, setRecentAndroidAddress, QString);
    property_copy(listenForAndroidDevicesOnStartup, setListenForAndroidDevicesOnStartup, bool);
#endif

    ////////////////////////////////
    //// File Version Detection ////
    ////////////////////////////////

    property_copy(detectModifiedFiles, setDetectModifiedFiles, bool);
    property_copy(modifiedFileComponent, setModifiedFileComponent, QString);
    property_copy(originalFileComponent, setOriginalFileComponent, QString);
    property_copy(moveOriginalContents, setMoveOriginalContents, bool);
    property_copy(autoStackNewFiles, setAutoStackNewFiles, bool);
    property_copy(copyFileComponent, setCopyFileComponent, QString);
    property_copy(copyFileReplacementComponent, setCopyFileReplacementComponent, QString);
    property_copy(loadOptimizationPreset, setLoadOptimizationPreset, int);
    property_copy(overlapLoadMD5, setOverlapLoadMD5, bool);
    property_copy(preloadThreadCount, setPreloadThreadCount, int);
    property_copy(thumbnailPreloadThreadCount, setThumbnailPreloadThreadCount, int);
    property_copy(thumbnailBuilderThreadCount, setThumbnailBuilderThreadCount, int);

    bool trustTimeStamps();

    ////////////////////
    //// Thumbnails ////
    ////////////////////

    property_copy(displayLabels, setDisplayLabels, bool);
    property_copy(displayCategories, setDisplayCategories, bool);
    property_copy(autoShowThumbnailView, setAutoShowThumbnailView, int);
    property_copy(showNewestThumbnailFirst, setShowNewestFirst, bool);
    property_copy(thumbnailDisplayGrid, setThumbnailDisplayGrid, bool);
    property_copy(previewSize, setPreviewSize, int);
    property_ref(colorScheme, setColorScheme, QString);
    property_copy(incrementalThumbnails, setIncrementalThumbnails, bool);

    // Border space around thumbnails.
    property_copy(thumbnailSpace, setThumbnailSpace, int);
    property_copy(thumbnailSize, setThumbnailSize, int);
    property_copy(minimumThumbnailSize, setMinimumThumbnailSize, int);
    property_copy(maximumThumbnailSize, setMaximumThumbnailSize, int);
    property_copy(actualThumbnailSize, setActualThumbnailSize, int);
    property_copy(thumbnailAspectRatio, setThumbnailAspectRatio, ThumbnailAspectRatio);

    ////////////////
    //// Viewer ////
    ////////////////

    property_ref(viewerSize, setViewerSize, QSize);
    property_ref(slideShowSize, setSlideShowSize, QSize);
    property_copy(launchViewerFullScreen, setLaunchViewerFullScreen, bool);
    property_copy(launchSlideShowFullScreen, setLaunchSlideShowFullScreen, bool);
    property_copy(showInfoBox, setShowInfoBox, bool);
    property_copy(showLabel, setShowLabel, bool);
    property_copy(showDescription, setShowDescription, bool);
    property_copy(showDate, setShowDate, bool);
    property_copy(showImageSize, setShowImageSize, bool);
    property_copy(showRating, setShowRating, bool);
    property_copy(showTime, setShowTime, bool);
    property_copy(showFilename, setShowFilename, bool);
    property_copy(showEXIF, setShowEXIF, bool);
    property_copy(smoothScale, setSmoothScale, bool);
    property_copy(slideShowInterval, setSlideShowInterval, int);
    property_copy(viewerCacheSize, setViewerCacheSize, int);
    property_copy(infoBoxWidth, setInfoBoxWidth, int);
    property_copy(infoBoxHeight, setInfoBoxHeight, int);
    property_copy(infoBoxPosition, setInfoBoxPosition, Position);
    property_copy(viewerStandardSize, setViewerStandardSize, StandardViewSize);

    ////////////////////
    //// Categories ////
    ////////////////////

    property_ref(untaggedCategory, setUntaggedCategory, QString);
    property_ref(untaggedTag, setUntaggedTag, QString);
    property_copy(untaggedImagesTagVisible, setUntaggedImagesTagVisible, bool);

    //////////////
    //// Exif ////
    //////////////

    property_ref(exifForViewer, setExifForViewer, StringSet);
    property_ref(exifForDialog, setExifForDialog, StringSet);
    property_ref(iptcCharset, setIptcCharset, QString);

    /////////////////////
    //// Exif Import ////
    /////////////////////

    property_copy(updateExifData, setUpdateExifData, bool);
    property_copy(updateImageDate, setUpdateImageDate, bool);
    property_copy(useModDateIfNoExif, setUseModDateIfNoExif, bool);
    property_copy(updateOrientation, setUpdateOrientation, bool);
    property_copy(updateDescription, setUpdateDescription, bool);

    ///////////////////////
    //// Miscellaneous ////
    ///////////////////////

    property_ref(HTMLBaseDir, setHTMLBaseDir, QString);
    property_ref(HTMLBaseURL, setHTMLBaseURL, QString);
    property_ref(HTMLDestURL, setHTMLDestURL, QString);
    property_ref(HTMLCopyright, setHTMLCopyright, QString);
    property_ref(HTMLDate, setHTMLDate, int);
    property_ref(HTMLTheme, setHTMLTheme, int);
    property_ref(HTMLKimFile, setHTMLKimFile, int);
    property_ref(HTMLInlineMovies, setHTMLInlineMovies, int);
    property_ref(HTML5Video, setHTML5Video, int);
    property_ref(HTML5VideoGenerate, setHTML5VideoGenerate, int);
    property_ref(HTMLThumbSize, setHTMLThumbSize, int);
    property_ref(HTMLNumOfCols, setHTMLNumOfCols, int);
    property_ref(HTMLSizes, setHTMLSizes, QString);
    property_ref(HTMLIncludeSelections, setHTMLIncludeSelections, QString);

    property_ref(fromDate, setFromDate, QDate);
    property_ref(toDate, setToDate, QDate);

    QString imageDirectory() const;

    QString groupForDatabase(const char *setting) const;

    DB::ImageSearchInfo currentLock() const;
    void setCurrentLock(const DB::ImageSearchInfo &, bool exclude);
    bool lockExcludes() const;

    bool locked() const;
    void setLocked(bool locked, bool force);

    void setWindowGeometry(WindowType, const QRect &geometry);
    QRect windowGeometry(WindowType) const;

    double getThumbnailAspectRatio() const;

    QStringList EXIFCommentsToStrip();
    void setEXIFCommentsToStrip(QStringList EXIFCommentsToStrip);

    bool getOverlapLoadMD5() const;
    int getPreloadThreadCount() const;
    int getThumbnailPreloadThreadCount() const;
    int getThumbnailBuilderThreadCount() const;

signals:
    void locked(bool lock, bool exclude);
    void viewSortTypeChanged(Settings::ViewSortType);
    void matchTypeChanged(AnnotationDialog::MatchType);
    void histogramSizeChanged(const QSize &);
    void thumbnailSizeChanged(int);
    void actualThumbnailSizeChanged(int);
    void histogramScaleChanged();
    void colorSchemeChanged();

private:
    SettingsData(const QString &imageDirectory);

    bool m_trustTimeStamps;
    bool m_hasAskedAboutTimeStamps;
    QString m_imageDirectory;
    static SettingsData *s_instance;

    friend class DB::CategoryCollection;

    QStringList m_EXIFCommentsToStrip;
};
} // end of namespace

#undef property
#undef property_copy
#undef property_ref

#endif /* SETTINGS_SETTINGSDATA_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
