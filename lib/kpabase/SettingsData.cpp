// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "SettingsData.h"

#include "Logging.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QStringList>
#include <QThread>

namespace
{
// when used from an application with different component name
// (e.g. kpa-thumbnailtool), we need to explicitly set the component name:
const QString configFile = QString::fromLatin1("kphotoalbumrc");
}
#define STR(x) QString::fromLatin1(x)

#define cfgValue(GROUP, OPTION, DEFAULT) \
    KSharedConfig::openConfig(configFile)->group(GROUP).readEntry(OPTION, DEFAULT)

#define setValue(GROUP, OPTION, VALUE)                                            \
    do {                                                                          \
        KConfigGroup group = KSharedConfig::openConfig(configFile)->group(GROUP); \
        group.writeEntry(OPTION, VALUE);                                          \
        group.sync();                                                             \
    } while (0)

#define getValueFunc_(TYPE, FUNC, GROUP, OPTION, DEFAULT)           \
    TYPE SettingsData::FUNC() const                                 \
    {                                                               \
        return static_cast<TYPE>(cfgValue(GROUP, OPTION, DEFAULT)); \
    }

#define setValueFunc_(FUNC, TYPE, GROUP, OPTION, VALUE) \
    void SettingsData::FUNC(const TYPE v)               \
    {                                                   \
        setValue(GROUP, OPTION, VALUE);                 \
    }

#define getValueFunc(TYPE, FUNC, GROUP, DEFAULT) getValueFunc_(TYPE, FUNC, #GROUP, #FUNC, DEFAULT)
#define setValueFunc(FUNC, TYPE, GROUP, OPTION) setValueFunc_(FUNC, TYPE, #GROUP, #OPTION, v)

// TODO(mfwitten): document parameters.
#define property_(GET_TYPE, GET_FUNC, GET_VALUE, SET_FUNC, SET_TYPE, SET_VALUE, GROUP, OPTION, GET_DEFAULT_1, GET_DEFAULT_2, GET_DEFAULT_2_TYPE) \
    GET_TYPE SettingsData::GET_FUNC() const                                                                                                      \
    {                                                                                                                                            \
        const KConfigGroup g = KSharedConfig::openConfig(configFile)->group(GROUP);                                                              \
                                                                                                                                                 \
        if (!g.hasKey(OPTION))                                                                                                                   \
            return GET_DEFAULT_1;                                                                                                                \
                                                                                                                                                 \
        GET_DEFAULT_2_TYPE v = g.readEntry(OPTION, static_cast<GET_DEFAULT_2_TYPE>(GET_DEFAULT_2));                                              \
        return static_cast<GET_TYPE>(GET_VALUE);                                                                                                 \
    }                                                                                                                                            \
    setValueFunc_(SET_FUNC, SET_TYPE, GROUP, OPTION, SET_VALUE)

#define property(GET_TYPE, GET_FUNC, SET_FUNC, SET_TYPE, SET_VALUE, GROUP, OPTION, GET_DEFAULT) \
    getValueFunc_(GET_TYPE, GET_FUNC, GROUP, OPTION, GET_DEFAULT)                               \
        setValueFunc_(SET_FUNC, SET_TYPE, GROUP, OPTION, SET_VALUE)

#define property_copy(GET_FUNC, SET_FUNC, TYPE, GROUP, GET_DEFAULT) \
    property(TYPE, GET_FUNC, SET_FUNC, TYPE, v, #GROUP, #GET_FUNC, GET_DEFAULT)

#define property_ref_(GET_FUNC, SET_FUNC, TYPE, GROUP, GET_DEFAULT) \
    property(TYPE, GET_FUNC, SET_FUNC, TYPE &, v, GROUP, #GET_FUNC, GET_DEFAULT)

#define property_ref(GET_FUNC, SET_FUNC, TYPE, GROUP, GET_DEFAULT) \
    property(TYPE, GET_FUNC, SET_FUNC, TYPE &, v, #GROUP, #GET_FUNC, GET_DEFAULT)

#define property_enum(GET_FUNC, SET_FUNC, TYPE, GROUP, GET_DEFAULT) \
    property(TYPE, GET_FUNC, SET_FUNC, TYPE, static_cast<int>(v), #GROUP, #GET_FUNC, static_cast<int>(GET_DEFAULT))

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define property_sset(GET_FUNC, SET_FUNC, GROUP, GET_DEFAULT) \
    property_(StringSet, GET_FUNC, (StringSet { v.begin(), v.end() }), SET_FUNC, StringSet &, (QStringList { v.begin(), v.end() }), #GROUP, #GET_FUNC, GET_DEFAULT, QStringList(), QStringList)
#else
#define property_sset(GET_FUNC, SET_FUNC, GROUP, GET_DEFAULT) \
    property_(StringSet, GET_FUNC, v.toSet(), SET_FUNC, StringSet &, v.toList(), #GROUP, #GET_FUNC, GET_DEFAULT, QStringList(), QStringList)
#endif

/**
 * smoothScale() is called from the image loading thread, therefore we need
 * to cache it this way, rather than going to KConfig.
 */
static bool _smoothScale = true;

using namespace Settings;

const WindowType Settings::MainWindow = "MainWindow";
const WindowType Settings::AnnotationDialog = "AnnotationDialog";

SettingsData *SettingsData::s_instance = nullptr;

SettingsData *SettingsData::instance()
{
    if (!s_instance)
        qFatal("SettingsData: instance called before loading a setup!");

    return s_instance;
}

bool SettingsData::ready()
{
    return s_instance;
}

void SettingsData::setup(const QString &imageDirectory, DB::UIDelegate &delegate)
{
    if (!s_instance)
        s_instance = new SettingsData(imageDirectory, delegate);
}

SettingsData::SettingsData(const QString &imageDirectory, DB::UIDelegate &delegate)
    : m_UI(delegate)
{
    m_hasAskedAboutTimeStamps = false;

    const QString s = STR("/");
    m_imageDirectory = imageDirectory.endsWith(s) ? imageDirectory : imageDirectory + s;

    _smoothScale = cfgValue("Viewer", "smoothScale", true);

    // Split the list of Exif comments that should be stripped automatically to a list

    QStringList commentsToStrip = cfgValue("General", "commentsToStrip", QString::fromLatin1("Exif_JPEG_PICTURE-,-OLYMPUS DIGITAL CAMERA-,-JENOPTIK DIGITAL CAMERA-,-")).split(QString::fromLatin1("-,-"), QString::SkipEmptyParts);
    for (QString &comment : commentsToStrip)
        comment.replace(QString::fromLatin1(",,"), QString::fromLatin1(","));

    m_EXIFCommentsToStrip = commentsToStrip;
}

/////////////////
//// General ////
/////////////////

// clang-format off
property_copy(useEXIFRotate, setUseEXIFRotate, bool, General, true)
property_copy(useEXIFComments, setUseEXIFComments, bool, General, true)
property_copy(stripEXIFComments, setStripEXIFComments, bool, General, true)
property_copy(commentsToStrip, setCommentsToStrip, QString, General, "" /* see constructor */)
property_copy(searchForImagesOnStart, setSearchForImagesOnStart, bool, General, true)
property_copy(ignoreFileExtension, setIgnoreFileExtension, bool, General, false)
property_copy(skipSymlinks, setSkipSymlinks, bool, General, false)
property_copy(skipRawIfOtherMatches, setSkipRawIfOtherMatches, bool, General, false)
property_copy(useRawThumbnail, setUseRawThumbnail, bool, General, true)
property_copy(useRawThumbnailSize, setUseRawThumbnailSize, QSize, General, QSize(1024, 768))
property_copy(useCompressedIndexXML, setUseCompressedIndexXML, bool, General, true)
property_copy(compressBackup, setCompressBackup, bool, General, true)
property_copy(showSplashScreen, setShowSplashScreen, bool, General, true)
property_copy(showHistogram, setShowHistogram, bool, General, true)
property_copy(autoSave, setAutoSave, int, General, 5)
property_copy(backupCount, setBackupCount, int, General, 5)
property_enum(tTimeStamps, setTTimeStamps, TimeStampTrust, General, Always)
property_copy(excludeDirectories, setExcludeDirectories, QString, General, QString::fromLatin1("xml,ThumbNails,.thumbs"))
#ifdef KPA_ENABLE_REMOTECONTROL
property_copy(recentAndroidAddress, setRecentAndroidAddress, QString, General, QString())
property_copy(listenForAndroidDevicesOnStartup, setListenForAndroidDevicesOnStartup, bool, General, false)
#endif
getValueFunc(QString, colorScheme, General, QString())
void SettingsData::setColorScheme(const QString &path) {
    if (path != colorScheme())
    {
        setValue("General", "colorScheme", path);
        emit colorSchemeChanged();
    }
}

getValueFunc(QSize, histogramSize, General, QSize(15, 30))
getValueFunc(ViewSortType, viewSortType, General, static_cast<int>(SortLastUse))
getValueFunc(AnnotationDialog::MatchType, matchType, General, static_cast<int>(AnnotationDialog::MatchFromWordStart))
getValueFunc(bool, histogramUseLinearScale, General, false)

    // clang-format on
    void SettingsData::setHistogramUseLinearScale(const bool useLinearScale)
{
    if (useLinearScale == histogramUseLinearScale())
        return;

    setValue("General", "histogramUseLinearScale", useLinearScale);
    emit histogramScaleChanged();
}

void SettingsData::setHistogramSize(const QSize &size)
{
    if (size == histogramSize())
        return;

    setValue("General", "histogramSize", size);
    emit histogramSizeChanged(size);
}

void SettingsData::setViewSortType(const ViewSortType tp)
{
    if (tp == viewSortType())
        return;

    setValue("General", "viewSortType", static_cast<int>(tp));
    emit viewSortTypeChanged(tp);
}
void SettingsData::setMatchType(const AnnotationDialog::MatchType mt)
{
    if (mt == matchType())
        return;

    setValue("General", "matchType", static_cast<int>(mt));
    emit matchTypeChanged(mt);
}

bool SettingsData::trustTimeStamps()
{
    if (tTimeStamps() == Always)
        return true;
    else if (tTimeStamps() == Never)
        return false;
    else {
        if (!m_hasAskedAboutTimeStamps) {
            const QString txt = i18n("When reading time information of images, their Exif info is used. "
                                     "Exif info may, however, not be supported by your KPhotoAlbum installation, "
                                     "or no valid information may be in the file. "
                                     "As a backup, KPhotoAlbum may use the timestamp of the image - this may, "
                                     "however, not be valid in case the image is scanned in. "
                                     "So the question is, should KPhotoAlbum trust the time stamp on your images?");
            const QString logMsg = QString::fromUtf8("Trust timestamps for this session?");
            auto answer = uiDelegate().questionYesNo(DB::LogMessage { BaseLog(), logMsg }, txt, i18n("Trust Time Stamps?"));
            if (answer == DB::UserFeedback::Confirm)
                m_trustTimeStamps = true;
            else
                m_trustTimeStamps = false;
            m_hasAskedAboutTimeStamps = true;
        }
        return m_trustTimeStamps;
    }
}

////////////////////////////////
//// File Version Detection ////
////////////////////////////////

// clang-format off
property_copy(detectModifiedFiles, setDetectModifiedFiles, bool, FileVersionDetection, true)
property_copy(modifiedFileComponent, setModifiedFileComponent, QString, FileVersionDetection, "^(.*)-edited.([^.]+)$")
property_copy(originalFileComponent, setOriginalFileComponent, QString, FileVersionDetection, "\\1.\\2")
property_copy(moveOriginalContents, setMoveOriginalContents, bool, FileVersionDetection, false)
property_copy(autoStackNewFiles, setAutoStackNewFiles, bool, FileVersionDetection, true)
property_copy(copyFileComponent, setCopyFileComponent, QString, FileVersionDetection, "(.[^.]+)$")
property_copy(copyFileReplacementComponent, setCopyFileReplacementComponent, QString, FileVersionDetection, "-edited\\1")
property_copy(loadOptimizationPreset, setLoadOptimizationPreset, int, FileVersionDetection, 0)
property_copy(overlapLoadMD5, setOverlapLoadMD5, bool, FileVersionDetection, false)
property_copy(preloadThreadCount, setPreloadThreadCount, int, FileVersionDetection, 1)
property_copy(thumbnailPreloadThreadCount, setThumbnailPreloadThreadCount, int, FileVersionDetection, 1)
property_copy(thumbnailBuilderThreadCount, setThumbnailBuilderThreadCount, int, FileVersionDetection, 0)
    // clang-format on

    ////////////////////
    //// Thumbnails ////
    ////////////////////

    // clang-format off
property_copy(displayLabels, setDisplayLabels, bool, Thumbnails, true)
property_copy(displayCategories, setDisplayCategories, bool, Thumbnails, false)
property_copy(autoShowThumbnailView, setAutoShowThumbnailView, int, Thumbnails, 20)
property_copy(showNewestThumbnailFirst, setShowNewestFirst, bool, Thumbnails, false)
property_copy(thumbnailDisplayGrid, setThumbnailDisplayGrid, bool, Thumbnails, false)
property_copy(previewSize, setPreviewSize, int, Thumbnails, 256)
property_copy(thumbnailSpace, setThumbnailSpace, int, Thumbnails, 4)
// not available via GUI, but should be consistent (and maybe confgurable for powerusers):
property_copy(minimumThumbnailSize, setMinimumThumbnailSize, int, Thumbnails, 32)
property_copy(maximumThumbnailSize, setMaximumThumbnailSize, int, Thumbnails, 4096)
property_enum(thumbnailAspectRatio, setThumbnailAspectRatio, ThumbnailAspectRatio, Thumbnails, Aspect_3_2)
property_copy(incrementalThumbnails, setIncrementalThumbnails, bool, Thumbnails, true)

// database specific so that changing it doesn't invalidate the thumbnail cache for other databases:
getValueFunc_(int, thumbnailSize, groupForDatabase("Thumbnails"), "thumbSize", 256)
    // clang-format on

    void SettingsData::setThumbnailSize(int value)
{
    // enforce limits:
    value = qBound(minimumThumbnailSize(), value, maximumThumbnailSize());

    if (value != thumbnailSize())
        emit thumbnailSizeChanged(value);
    setValue(groupForDatabase("Thumbnails"), "thumbSize", value);
}

int SettingsData::actualThumbnailSize() const
{
    // this is database specific since it's a derived value of thumbnailSize
    int retval = cfgValue(groupForDatabase("Thumbnails"), "actualThumbSize", 0);
    // if no value has been set, use thumbnailSize
    if (retval == 0)
        retval = thumbnailSize();
    return retval;
}

void SettingsData::setActualThumbnailSize(int value)
{
    // enforce limits:
    value = qBound(minimumThumbnailSize(), value, thumbnailSize());

    if (value != actualThumbnailSize()) {
        setValue(groupForDatabase("Thumbnails"), "actualThumbSize", value);
        emit actualThumbnailSizeChanged(value);
    }
}

////////////////
//// Viewer ////
////////////////

// clang-format off
property_ref(viewerSize, setViewerSize, QSize, Viewer, QSize(1024, 768))
property_ref(slideShowSize, setSlideShowSize, QSize, Viewer, QSize(1024, 768))
property_copy(launchViewerFullScreen, setLaunchViewerFullScreen, bool, Viewer, false)
property_copy(launchSlideShowFullScreen, setLaunchSlideShowFullScreen, bool, Viewer, true)
property_copy(showInfoBox, setShowInfoBox, bool, Viewer, true)
property_copy(showLabel, setShowLabel, bool, Viewer, true)
property_copy(showDescription, setShowDescription, bool, Viewer, true)
property_copy(showDate, setShowDate, bool, Viewer, true)
property_copy(showImageSize, setShowImageSize, bool, Viewer, true)
property_copy(showRating, setShowRating, bool, Viewer, true)
property_copy(showTime, setShowTime, bool, Viewer, true)
property_copy(showFilename, setShowFilename, bool, Viewer, false)
property_copy(showEXIF, setShowEXIF, bool, Viewer, true)
property_copy(slideShowInterval, setSlideShowInterval, int, Viewer, 5)
property_copy(viewerCacheSize, setViewerCacheSize, int, Viewer, 195)
property_copy(infoBoxWidth, setInfoBoxWidth, int, Viewer, 400)
property_copy(infoBoxHeight, setInfoBoxHeight, int, Viewer, 300)
property_enum(infoBoxPosition, setInfoBoxPosition, Position, Viewer, Bottom)
property_enum(viewerStandardSize, setViewerStandardSize, StandardViewSize, Viewer, FullSize)
property_enum(videoBackend, setVideoBackend, VideoBackend, Viewer, VideoBackend::NotConfigured)
    // clang-format on

    bool SettingsData::smoothScale() const
{
    return _smoothScale;
}

void SettingsData::setSmoothScale(bool b)
{
    _smoothScale = b;
    setValue("Viewer", "smoothScale", b);
}

////////////////////
//// Categories ////
////////////////////

// clang-format off
property_ref(untaggedCategory, setUntaggedCategory, QString, General, i18n("Events"))
property_ref(untaggedTag, setUntaggedTag, QString, General, i18n("untagged"))
property_copy(untaggedImagesTagVisible, setUntaggedImagesTagVisible, bool, General, false)
    // clang-format on

    //////////////
    //// Exif ////
    //////////////

    // clang-format off
property_sset(exifForViewer, setExifForViewer, Exif, StringSet())
property_sset(exifForDialog, setExifForDialog, Exif, StringSet())
property_ref(iptcCharset, setIptcCharset, QString, Exif, QString())
    // clang-format on

    /////////////////////
    //// Exif Import ////
    /////////////////////

    // clang-format off
property_copy(updateExifData, setUpdateExifData, bool, ExifImport, true)
property_copy(updateImageDate, setUpdateImageDate, bool, ExifImport, false)
property_copy(useModDateIfNoExif, setUseModDateIfNoExif, bool, ExifImport, true)
property_copy(updateOrientation, setUpdateOrientation, bool, ExifImport, false)
property_copy(updateDescription, setUpdateDescription, bool, ExifImport, false)
    // clang-format on

    ///////////////////////
    //// Miscellaneous ////
    ///////////////////////

    // clang-format off
property_ref_(HTMLBaseDir, setHTMLBaseDir, QString, groupForDatabase("HTML Settings"), QString::fromLocal8Bit(qgetenv("HOME")) + STR("/public_html"))
property_ref_(HTMLBaseURL, setHTMLBaseURL, QString, groupForDatabase("HTML Settings"), STR("file://") + HTMLBaseDir())
property_ref_(HTMLDestURL, setHTMLDestURL, QString, groupForDatabase("HTML Settings"), STR("file://") + HTMLBaseDir())
property_ref_(HTMLCopyright, setHTMLCopyright, QString, groupForDatabase("HTML Settings"), STR(""))
property_ref_(HTMLDate, setHTMLDate, int, groupForDatabase("HTML Settings"), true)
property_ref_(HTMLTheme, setHTMLTheme, int, groupForDatabase("HTML Settings"), -1)
property_ref_(HTMLKimFile, setHTMLKimFile, int, groupForDatabase("HTML Settings"), true)
property_ref_(HTMLInlineMovies, setHTMLInlineMovies, int, groupForDatabase("HTML Settings"), true)
property_ref_(HTML5Video, setHTML5Video, int, groupForDatabase("HTML Settings"), true)
property_ref_(HTML5VideoGenerate, setHTML5VideoGenerate, int, groupForDatabase("HTML Settings"), true)
property_ref_(HTMLThumbSize, setHTMLThumbSize, int, groupForDatabase("HTML Settings"), 128)
property_ref_(HTMLNumOfCols, setHTMLNumOfCols, int, groupForDatabase("HTML Settings"), 5)
property_ref_(HTMLSizes, setHTMLSizes, QString, groupForDatabase("HTML Settings"), STR(""))
property_ref_(HTMLIncludeSelections, setHTMLIncludeSelections, QString, groupForDatabase("HTML Settings"), STR(""))
    // clang-format on

    QDate SettingsData::fromDate() const
{
    QString date = cfgValue("Miscellaneous", "fromDate", STR(""));
    return date.isEmpty() ? QDate(QDate::currentDate().year(), 1, 1) : QDate::fromString(date, Qt::ISODate);
}

void SettingsData::setFromDate(const QDate &date)
{
    if (date.isValid())
        setValue("Miscellaneous", "fromDate", date.toString(Qt::ISODate));
}

QDate SettingsData::toDate() const
{
    QString date = cfgValue("Miscellaneous", "toDate", STR(""));
    return date.isEmpty() ? QDate(QDate::currentDate().year() + 1, 1, 1) : QDate::fromString(date, Qt::ISODate);
}

void SettingsData::setToDate(const QDate &date)
{
    if (date.isValid())
        setValue("Miscellaneous", "toDate", date.toString(Qt::ISODate));
}

QString SettingsData::imageDirectory() const
{
    return m_imageDirectory;
}

QString SettingsData::groupForDatabase(const char *setting) const
{
    return STR("%1 - %2").arg(STR(setting)).arg(imageDirectory());
}

QVariantMap SettingsData::currentLock() const
{
    // duplicating logic from ImageSearchInfo here is not ideal
    // FIXME(jzarl): review the whole database view lock mechanism
    const auto group = groupForDatabase("Privacy Settings");
    QVariantMap keyValuePairs;
    keyValuePairs[STR("label")] = cfgValue(group, "label", {});
    keyValuePairs[STR("description")] = cfgValue(group, "description", {});
    // reading a QVariant containing a stringlist is asking too much of cfgValue:
    const auto config = KSharedConfig::openConfig(configFile)->group(group);
    const QStringList categories = config.readEntry<QStringList>(QString::fromUtf8("categories"), QStringList());
    keyValuePairs[STR("categories")] = QVariant(categories);
    for (QStringList::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it) {
        keyValuePairs[*it] = cfgValue(group, *it, {});
    }
    return keyValuePairs;
}

void SettingsData::setCurrentLock(const QVariantMap &pairs, bool exclude)
{
    for (QVariantMap::const_iterator it = pairs.cbegin(); it != pairs.cend(); ++it) {
        setValue(groupForDatabase("Privacy Settings"), it.key(), it.value());
    }
    setValue(groupForDatabase("Privacy Settings"), "exclude", exclude);
}

bool SettingsData::lockExcludes() const
{
    return cfgValue(groupForDatabase("Privacy Settings"), "exclude", false);
}

getValueFunc_(bool, locked, groupForDatabase("Privacy Settings"), "locked", false)

    void SettingsData::setLocked(bool lock, bool force)
{
    if (lock == locked() && !force)
        return;

    setValue(groupForDatabase("Privacy Settings"), "locked", lock);
    emit locked(lock, lockExcludes());
}

void SettingsData::setWindowGeometry(WindowType win, const QRect &geometry)
{
    setValue("Window Geometry", win, geometry);
}

QRect SettingsData::windowGeometry(WindowType win) const
{
    return cfgValue("Window Geometry", win, QRect());
}

double Settings::SettingsData::getThumbnailAspectRatio() const
{
    double ratio = 1.0;
    switch (Settings::SettingsData::instance()->thumbnailAspectRatio()) {
    case Settings::Aspect_16_9:
        ratio = 9.0 / 16;
        break;
    case Settings::Aspect_4_3:
        ratio = 3.0 / 4;
        break;
    case Settings::Aspect_3_2:
        ratio = 2.0 / 3;
        break;
    case Settings::Aspect_9_16:
        ratio = 16 / 9.0;
        break;
    case Settings::Aspect_3_4:
        ratio = 4 / 3.0;
        break;
    case Settings::Aspect_2_3:
        ratio = 3 / 2.0;
        break;
    case Settings::Aspect_1_1:
        ratio = 1.0;
        break;
    }
    return ratio;
}

QStringList Settings::SettingsData::EXIFCommentsToStrip()
{
    return m_EXIFCommentsToStrip;
}

void Settings::SettingsData::setEXIFCommentsToStrip(QStringList EXIFCommentsToStrip)
{
    m_EXIFCommentsToStrip = EXIFCommentsToStrip;
}

bool Settings::SettingsData::getOverlapLoadMD5() const
{
    switch (Settings::SettingsData::instance()->loadOptimizationPreset()) {
    case Settings::LoadOptimizationSlowNVME:
    case Settings::LoadOptimizationFastNVME:
        return true;
    case Settings::LoadOptimizationManual:
        return Settings::SettingsData::instance()->overlapLoadMD5();
    case Settings::LoadOptimizationHardDisk:
    case Settings::LoadOptimizationNetwork:
    case Settings::LoadOptimizationSataSSD:
    default:
        return false;
    }
}

int Settings::SettingsData::getPreloadThreadCount() const
{
    switch (Settings::SettingsData::instance()->loadOptimizationPreset()) {
    case Settings::LoadOptimizationManual:
        return Settings::SettingsData::instance()->preloadThreadCount();
    case Settings::LoadOptimizationSlowNVME:
    case Settings::LoadOptimizationFastNVME:
    case Settings::LoadOptimizationSataSSD:
        return qMax(1, qMin(16, QThread::idealThreadCount()));
    case Settings::LoadOptimizationHardDisk:
    case Settings::LoadOptimizationNetwork:
    default:
        return 1;
    }
}

int Settings::SettingsData::getThumbnailPreloadThreadCount() const
{
    switch (Settings::SettingsData::instance()->loadOptimizationPreset()) {
    case Settings::LoadOptimizationManual:
        return Settings::SettingsData::instance()->thumbnailPreloadThreadCount();
    case Settings::LoadOptimizationSlowNVME:
    case Settings::LoadOptimizationFastNVME:
    case Settings::LoadOptimizationSataSSD:
        return qMax(1, qMin(16, QThread::idealThreadCount() / 2));
    case Settings::LoadOptimizationHardDisk:
    case Settings::LoadOptimizationNetwork:
    default:
        return 1;
    }
}

int Settings::SettingsData::getThumbnailBuilderThreadCount() const
{
    switch (Settings::SettingsData::instance()->loadOptimizationPreset()) {
    case Settings::LoadOptimizationManual:
        return Settings::SettingsData::instance()->thumbnailBuilderThreadCount();
    case Settings::LoadOptimizationSlowNVME:
    case Settings::LoadOptimizationFastNVME:
    case Settings::LoadOptimizationSataSSD:
    case Settings::LoadOptimizationHardDisk:
    case Settings::LoadOptimizationNetwork:
    default:
        return qMax(1, qMin(16, QThread::idealThreadCount() - 1));
    }
}

DB::UIDelegate &SettingsData::uiDelegate() const
{
    return m_UI;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
