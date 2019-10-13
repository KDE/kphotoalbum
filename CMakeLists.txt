cmake_minimum_required(VERSION 3.3.0)
project(kphotoalbum LANGUAGES CXX VERSION 5.7.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# provide drop-down menu for build-type in cmake-gui:
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ";Debug;Release;RelWithDebInfo;MinSizeRel")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}" )

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

# enable exceptions:
kde_enable_exceptions()

add_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_DEPRECATED_WARNINGS
    -DQT_STRICT_ITERATORS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
    -DQT_DISABLE_Q_FOREACH
    )

########### dependencies  ###############

find_package(Qt5 5.9 REQUIRED COMPONENTS Core Sql Xml Widgets Test)
find_package(Phonon4Qt5 REQUIRED)
find_package(KF5 5.44 REQUIRED COMPONENTS Archive Completion Config CoreAddons DocTools I18n IconThemes JobWidgets KIO TextWidgets XmlGui WidgetsAddons)

find_package(JPEG REQUIRED)
if(JPEG_FOUND)
    include_directories("${JPEG_INCLUDE_DIR}")
endif()

### 2018-12-30 jzarl
# When Exiv2 0.26 can be deprecated, FindExiv2.cmake should be removed
# and only find_package(exiv2) should be used
find_package(exiv2 CONFIG QUIET)
if(exiv2_FOUND)
    # search againg with REQUIRED, so that the feature summary correctly shows exiv as required dependency
    find_package(exiv2 CONFIG REQUIRED)
    set(EXIV2_LIBRARIES exiv2lib)
else()
    find_package(Exiv2 REQUIRED)
endif()

find_package(KF5Purpose)
set_package_properties(KF5Purpose
    PROPERTIES
    TYPE RECOMMENDED
    PURPOSE "Enables integration with KDE Purpose plugins, which provide image sharing and similar functionality."
    )

find_package(KF5KDcraw)
set_package_properties(KF5KDcraw
    PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Enables RAW image support"
    )
set(HAVE_KDCRAW ${KF5KDcraw_FOUND} )

find_package(Marble)
set_package_properties(Marble
    PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Enables support for geographic map location using embedded GPS information."
    )
set(HAVE_MARBLE ${Marble_FOUND})
if(Marble_FOUND)
    include(MarbleChecks)
endif()

add_custom_target(
    UpdateVersion ALL
    COMMAND "${CMAKE_COMMAND}"
    "-DBASE_DIR=${CMAKE_CURRENT_SOURCE_DIR}" -DPROJECT_NAME=KPA -DPROJECT_VERSION="${PROJECT_VERSION}"
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/UpdateVersion.cmake"
    COMMENT "Updating version header."
    BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/version.h"
    )

# For config-kpa-*.h
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

set(libdatebar_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/DateBarWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/ViewHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.cpp"
    )

set(libSettings_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsData.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerSizeConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/TagGroupsPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/GeneralPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/FileVersionDetectionPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ThumbnailsPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/DatabaseBackendPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/UntaggedGroupBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoriesGroupsWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/BirthdayPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/DateTableWidgetItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/Logging.cpp"
    )

set(libxmldb_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/Database.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XMLCategoryCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XMLCategory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XMLImageDateCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/NumberedBackup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/FileReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/FileWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/ElementWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XmlReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/CompressFileInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/Logging.cpp"
    )

set(libThumbnailView_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/FilterWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailToolTip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeSlider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseTrackingInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/CellGeometry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailFacade.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailComponent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/KeyboardEventHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailDND.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/Delegate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionMaintainer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/VideoThumbnailCycler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailFactory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/enums.cpp"
    )

set(libPlugins_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Logging.cpp"
    )
if(KF5Purpose_FOUND)
    set(libPlugins_SRCS ${libPlugins_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PurposeMenu.cpp"
        )
endif()

set(libViewer_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewerWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ImageDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/SpeedDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CategoryImageConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/AbstractDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TextDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBoxResizer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VisibleOptionsMenu.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoShooter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TaggedArea.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/RemoteVideoDisplayController.cpp"
    )

set(libCategoryListView_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragableTreeWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/CheckDropItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragItemInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/Logging.cpp"
    )

set(libHTMLGenerator_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/HTMLDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Generator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Setup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/ImageSizeCheckBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/ImageSizeCheckBox.cpp"
    )

set(libUtilities_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/AlgorithmHelper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ShowBusyCursor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/List.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/UniqFilenameMapper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FileUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/BooleanGuard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Process.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DeleteFiles.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ToolTip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/JpeglibWithFix.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/StringSet.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FastJpeg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DemoUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DescriptionUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FileNameUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/VideoUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ImageUtil.cpp"
    )

set(libMainWindow_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DeleteDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/RunDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/FeatureDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/InvalidDateFinder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/AutoStackImages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/TokenEditor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/WelcomeDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SplashScreen.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ExternalPopup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/CategoryImagePopup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SearchBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ImageCounter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DirtyIndicator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatisticsDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/BreadcrumbViewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatusBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/UpdateVideoThumbnail.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/DuplicateMerger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/DuplicateMatch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/MergeToolTip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/CopyPopup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Options.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Logging.cpp"
    )

set(libImageManager_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageLoaderThread.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/AsyncLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageClientInterface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageDecoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RawImageDecoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RequestQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ThumbnailCache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageEvent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ThumbnailBuilder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/PreloadRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/CancelEvent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoImageRescaleRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoThumbnails.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoLengthExtractor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ExtractOneVideoFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/CacheFileInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/enums.cpp"
    )

set(libDB_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/Category.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ExactCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5Map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MemberMap.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDB.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FileInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/NegationCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/NewImageFinder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageScout.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/NoTagCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/GroupCounter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageSearchInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ContainerCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ValueCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/OrCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/AndCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FastDir.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/OptimizedFileList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FileName.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FileNameList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryPtr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ExifMode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDateCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoPtr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MediaCount.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/RawId.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/SimpleCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/UIDelegate.cpp"
    )

set(libImportExport_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Export.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Import.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/XMLHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MiniViewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImageRow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportSettings.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/KimFileReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MD5CheckPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Logging.cpp"
    )

set(libAnnotationDialog_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Dialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListSelect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreviewWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/DateEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/CompletableLineEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListViewItemHider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShowSelectionOnlyManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShortCutManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ResizableFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/DescriptionEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/AreaTagSelectDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/enums.cpp"
    )

set(libBrowser_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/OverviewPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/CategoryPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/ImageViewPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/TreeFilter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/Breadcrumb.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BreadcrumbList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/AbstractCategoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/FlatCategoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/TreeCategoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/CenteringIconView.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/enums.cpp"
    )

set(libExif_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/Database.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/InfoDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/TreeView.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/Info.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/RangeWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/DatabaseElement.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/ReReadDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/Grid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialogSettings.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ExifPage.cpp"
    )

set(libBackgroundTaskManager_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobInterface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/StatusIndicator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobViewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/CompletedJobInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/Priority.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/PriorityQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/Logging.cpp"
    )

set(libBackgroundJobs_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/SearchForVideosWithoutLengthInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/ReadVideoLengthJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/SearchForVideosWithoutVideoThumbnailsJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/HandleVideoThumbnailRequestJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/ExtractOneThumbnailJob.cpp"
    )

option(KPA_ENABLE_REMOTECONTROL "Build with support for companion Android/QML app." OFF)
set(libRemoteControl_SRCS)
set(libRemoteControl_LIBS)
if(KPA_ENABLE_REMOTECONTROL)
    find_package(Qt5 5.9 REQUIRED COMPONENTS Network)
    # requires cmake 3.12
    add_compile_definitions(KPA_ENABLE_REMOTECONTROL)
    set(libRemoteControl_LIBS Qt5::Network)
    set(libRemoteControl_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteCommand.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteConnection.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/Server.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteInterface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/SearchInfo.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteImageRequest.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/ImageNameStore.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/ConnectionIndicator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/Logging.cpp"
        )
endif()

set(libMap_SRCS)
if(Marble_FOUND)
    set(libMap_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser/GeoPositionPage.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/GeoCluster.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/MapView.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/Logging.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/GeoCoordinates.cpp"
        )
endif()

add_subdirectory(images)
add_subdirectory(icons)
add_subdirectory(demo)
add_subdirectory(themes)
add_subdirectory(scripts)
add_subdirectory(doc)


########### next target ###############

set(kphotoalbum_SRCS
    main.cpp
    ${libdatebar_SRCS}
    ${libSettings_SRCS}
    ${libsurvey_SRCS}
    ${libxmldb_SRCS}
    ${libThumbnailView_SRCS}
    ${libPlugins_SRCS}
    ${libViewer_SRCS}
    ${libCategoryListView_SRCS}
    ${libHTMLGenerator_SRCS}
    ${libMainWindow_SRCS}
    ${libImageManager_SRCS}
    ${libDB_SRCS}
    ${libImportExport_SRCS}
    ${libAnnotationDialog_SRCS}
    ${libExif_SRCS}
    ${libBrowser_SRCS}
    ${libBackgroundTaskManager_SRCS}
    ${libBackgroundJobs_SRCS}
    ${libRemoteControl_SRCS}
    ${libMap_SRCS}
    ${libUtilities_SRCS}
    )

add_executable(kphotoalbum ${kphotoalbum_SRCS})
add_dependencies(kphotoalbum UpdateVersion)

# External components
target_link_libraries(kphotoalbum
    PRIVATE
    ${JPEG_LIBRARY}
    ${EXIV2_LIBRARIES}
    KF5::Archive
    KF5::Completion
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::JobWidgets
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::TextWidgets
    KF5::XmlGui
    KF5::WidgetsAddons
    Phonon::phonon4qt5
    ${libRemoteControl_LIBS}
    Qt5::Sql
    )

if(KF5Purpose_FOUND)
    target_link_libraries(kphotoalbum PRIVATE KF5::Purpose KF5::PurposeWidgets)
endif()

if(KF5KDcraw_FOUND)
    target_link_libraries(kphotoalbum PRIVATE KF5::KDcraw)
endif()

if(Marble_FOUND)
    target_link_libraries(kphotoalbum PRIVATE Marble)
endif()

install(TARGETS kphotoalbum ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_library(kpabase
    AnnotationDialog/enums.cpp
    AnnotationDialog/enums.h
    DB/FileName.cpp
    DB/FileName.h
    DB/FileNameList.cpp
    DB/FileNameList.h
    DB/Logging.cpp
    DB/Logging.h
    DB/UIDelegate.cpp
    DB/UIDelegate.h
    ImageManager/Logging.cpp
    ImageManager/Logging.h
    MainWindow/Logging.cpp
    MainWindow/Logging.h
    Settings/SettingsData.cpp
    Settings/SettingsData.h
    Utilities/FileNameUtil.cpp
    Utilities/FileNameUtil.h
    Utilities/StringSet.cpp
    Utilities/StringSet.h
    )
add_library(KPA::Base ALIAS kpabase)
target_link_libraries(kpabase
    PUBLIC
    KF5::ConfigCore
    KF5::I18n
    Qt5::Core
    )
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpabase PROPERTIES CXX_VISIBILITY_PRESET default)
add_library(kpathumbnails
    ImageManager/Logging.cpp
    ImageManager/Logging.h
    ImageManager/ThumbnailCache.cpp
    ImageManager/ThumbnailCache.h
    )
add_library(KPA::Thumbnails ALIAS kpathumbnails)
# FIXME(jzarl): once KPhotoAlbum uses internal libs, export public symbols instead:
set_target_properties(kpathumbnails PROPERTIES CXX_VISIBILITY_PRESET default)
target_link_libraries(kpathumbnails
    PUBLIC
    KPA::Base
    Qt5::Gui
    )
install(TARGETS kpabase kpathumbnails ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(kpa-thumbnailtool)

########### install files ###############

install(PROGRAMS org.kde.kphotoalbum.desktop org.kde.kphotoalbum-import.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kphotoalbumrc DESTINATION ${KDE_INSTALL_CONFDIR})
install(FILES tips default-setup DESTINATION "${KDE_INSTALL_DATADIR}/kphotoalbum")
install(FILES kphotoalbumui.rc DESTINATION "${KDE_INSTALL_KXMLGUI5DIR}/kphotoalbum")
install(FILES org.kde.kphotoalbum.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

configure_file(config-kpa-kdcraw.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-kdcraw.h")
configure_file(config-kpa-plugins.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-plugins.h")
configure_file(config-kpa-marble.h.cmake "${CMAKE_CURRENT_BINARY_DIR}/config-kpa-marble.h")

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# vi:expandtab:tabstop=4 shiftwidth=4:
