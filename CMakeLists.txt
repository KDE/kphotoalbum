# SPDX-FileCopyrightText: 2007 Gustavo P. Boiko <gustavo.boiko@kdemail.net>
# SPDX-FileCopyrightText: 2007-2008 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2007-2010 Tuomas Suutari <tuomas@nepnep.net>
# SPDX-FileCopyrightText: 2007-2023 Jesper K. Pedersen <blackie@kde.org>
# SPDX-FileCopyrightText: 2008, 2013-2014, 2016 Pino Toscano <pino@kde.org>
# SPDX-FileCopyrightText: 2008-2009 Henner Zeller <h.zeller@acm.org>
# SPDX-FileCopyrightText: 2008-2009 Jan Kundrát <jkt@flaska.net>
# SPDX-FileCopyrightText: 2009 Hassan Ibraheem <hasan.ibraheem@gmail.com>
# SPDX-FileCopyrightText: 2010, 2012 Miika Turkia <miika.turkia@gmail.com>
# SPDX-FileCopyrightText: 2010, 2015, 2019 David Faure <faure@kde.org>
# SPDX-FileCopyrightText: 2012 Albert Astals Cid <tsdgeos@terra.es>
# SPDX-FileCopyrightText: 2012-2013 Andreas Neustifter <andreas.neustifter@gmail.com>
# SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
# SPDX-FileCopyrightText: 2016 Christoph Feck <cfeck@kde.org>
# SPDX-FileCopyrightText: 2016 Luigi Toscano <luigi.toscano@tiscali.it>
# SPDX-FileCopyrightText: 2016 Michael Palimaka <kensington@gentoo.org>
# SPDX-FileCopyrightText: 2017 Ben Cooksley <bcooksley@kde.org>
# SPDX-FileCopyrightText: 2018, 2020 Robert Krawitz <rlk@alum.mit.edu>
# SPDX-FileCopyrightText: 2020 Wes Hardaker <kpa@capturedonearth.com>
# SPDX-FileCopyrightText: 2012-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2014-2025 Tobias Leupold <tl@stonemx.de>
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.30.0)
project(kphotoalbum LANGUAGES CXX VERSION 6.0.1)

# set minimum versions here:
set(MINIMUM_KF6_VERSION 6.0.0)
set(MINIMUM_QT6_VERSION_MAJOR 6)
set(MINIMUM_QT6_VERSION_MINOR 6)
set(MINIMUM_QT6_VERSION_PATCH 0)
set(MINIMUM_QT6_VERSION "${MINIMUM_QT6_VERSION_MAJOR}.${MINIMUM_QT6_VERSION_MINOR}.${MINIMUM_QT6_VERSION_PATCH}")
math(EXPR MINIMUM_QT6_VERSION_HEX "${MINIMUM_QT6_VERSION_MAJOR} << 24 | ${MINIMUM_QT6_VERSION_MINOR} << 16 | ${MINIMUM_QT6_VERSION_PATCH}" OUTPUT_FORMAT HEXADECIMAL)
set(KDE_COMPILERSETTINGS_LEVEL 6.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)

# provide drop-down menu for build-type in cmake-gui:
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ";Debug;Release;RelWithDebInfo;MinSizeRel")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(ECM ${MINIMUM_KF6_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

# enable exceptions:
kde_enable_exceptions()

file(GLOB_RECURSE PRE_COMMIT_HOOKS ${CMAKE_SOURCE_DIR}/dev/githooks/pre-commit.d/*)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT CUSTOM_SCRIPTS ${PRE_COMMIT_HOOKS})

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=${MINIMUM_QT6_VERSION_HEX}
    )

########### dependencies  ###############

find_package(Qt6 ${MINIMUM_QT6_VERSION} REQUIRED COMPONENTS Core Sql Xml Widgets DBus Test)
find_package(Phonon4Qt6)
find_package(LIBVLC)
find_package(KF6 ${MINIMUM_KF6_VERSION} REQUIRED COMPONENTS Archive Completion Config CoreAddons I18n IconThemes JobWidgets KIO TextWidgets XmlGui WidgetsAddons ColorScheme)

find_package(JPEG REQUIRED)
if(JPEG_FOUND)
    include_directories("${JPEG_INCLUDE_DIR}")
endif()

find_package(KF6Purpose)
set_package_properties(KF6Purpose
    PROPERTIES
    TYPE RECOMMENDED
    PURPOSE "Enables integration with KDE Purpose plugins, which provide image sharing and similar functionality."
    )

find_package(KDcrawQt6)
set_package_properties(KDcrawQt6
    PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Enables RAW image support"
    )
set(HAVE_KDCRAW ${KDcrawQt6_FOUND})

find_package(Marble 24.11.70)
set_package_properties(Marble
    PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Enables support for geographic map location using embedded GPS information."
    )
set(HAVE_MARBLE ${Marble_FOUND})

if(NOT Phonon4Qt6_FOUND AND NOT LIBVLC_FOUND)
    message(FATAL_ERROR "A video backend must be provided. CMake failed to find any of Phonon and VLC")
endif()

set(libdatebar_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/DateBarWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/DateBarWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/ViewHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/ViewHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.h"
    )

set(libSettings_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerSizeConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerSizeConfig.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryItem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/TagGroupsPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/TagGroupsPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/GeneralPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/GeneralPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/FileVersionDetectionPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/FileVersionDetectionPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ThumbnailsPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ThumbnailsPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/DatabaseBackendPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/DatabaseBackendPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/UntaggedGroupBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/UntaggedGroupBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoriesGroupsWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoriesGroupsWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/BirthdayPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/BirthdayPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/DateTableWidgetItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/DateTableWidgetItem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/Logging.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/VideoPlayerSelectorDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/VideoPlayerSelectorDialog.h"
    )

set(libThumbnailView_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/FilterWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/FilterWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailRequest.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailToolTip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailToolTip.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeInteraction.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeSlider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeSlider.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionInteraction.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseTrackingInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseTrackingInteraction.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/CellGeometry.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/CellGeometry.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailModel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailFacade.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailFacade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailComponent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailComponent.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/KeyboardEventHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/KeyboardEventHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailDND.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailDND.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/Delegate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/Delegate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionMaintainer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionMaintainer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/VideoThumbnailCycler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/VideoThumbnailCycler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/Logging.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseInteraction.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseInteraction.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailFactory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailFactory.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/enums.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/enums.h"
    )

set(libPlugins_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Logging.h"
    )
if(KF6Purpose_FOUND)
    set(libPlugins_SRCS ${libPlugins_SRCS}
        "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PurposeMenu.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PurposeMenu.h"
        )
endif()

set(libViewer_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewerWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewerWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ImageDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ImageDisplay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TransientDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TransientDisplay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CategoryImageConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CategoryImageConfig.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/AbstractDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/AbstractDisplay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoDisplay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TextDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TextDisplay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBoxResizer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBoxResizer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VisibleOptionsMenu.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VisibleOptionsMenu.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoShooter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoShooter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TaggedArea.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TaggedArea.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Logging.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoToolBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoToolBar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Slider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Slider.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CursorVisibilityHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CursorVisibilityHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/AnnotationHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/AnnotationHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/SelectCategoryAndValue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/SelectCategoryAndValue.h"
    )

ki18n_wrap_ui(libViewer_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/SelectCategoryAndValue.ui"
    )


set(libCategoryListView_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragableTreeWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragableTreeWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/CheckDropItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/CheckDropItem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragItemInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragItemInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/Logging.h"
    )

set(libHTMLGenerator_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/HTMLDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/HTMLDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Generator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Generator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Setup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Setup.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/ImageSizeCheckBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/ImageSizeCheckBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Logging.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/ImageSizeCheckBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/ImageSizeCheckBox.h"
    )

set(libUtilities_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/AlgorithmHelper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/AlgorithmHelper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ShowBusyCursor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ShowBusyCursor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/List.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/List.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/UniqFilenameMapper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/UniqFilenameMapper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/BooleanGuard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/BooleanGuard.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Process.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Process.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DeleteFiles.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DeleteFiles.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ToolTip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ToolTip.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/JpeglibWithFix.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/JpeglibWithFix.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FastJpeg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FastJpeg.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DemoUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DemoUtil.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DescriptionUtil.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/DescriptionUtil.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FastDateTime.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/FastDateTime.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Timespan.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Timespan.h"
    )

set(libMainWindow_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DeleteDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DeleteDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/RunDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/RunDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/FeatureDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/FeatureDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/InvalidDateFinder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/InvalidDateFinder.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/AutoStackImages.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/AutoStackImages.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/TokenEditor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/TokenEditor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/WelcomeDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/WelcomeDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Window.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SplashScreen.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SplashScreen.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ExternalPopup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ExternalPopup.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/CategoryImagePopup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/CategoryImagePopup.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SearchBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SearchBar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ImageCounter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ImageCounter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DirtyIndicator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DirtyIndicator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatisticsDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatisticsDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/BreadcrumbViewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/BreadcrumbViewer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatusBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatusBar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/UpdateVideoThumbnail.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/UpdateVideoThumbnail.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/DuplicateMerger.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/DuplicateMerger.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/DuplicateMatch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/DuplicateMatch.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/MergeToolTip.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DuplicateMerger/MergeToolTip.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/CopyLinkEngine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/CopyLinkEngine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Options.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Options.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Logging.h"
    )

set(libImageManager_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageLoaderThread.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageLoaderThread.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/AsyncLoader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/AsyncLoader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageRequest.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageClientInterface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageClientInterface.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageDecoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageDecoder.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RawImageDecoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RawImageDecoder.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RequestQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RequestQueue.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageEvent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageEvent.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ThumbnailBuilder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ThumbnailBuilder.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/PreloadRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/PreloadRequest.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/CancelEvent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/CancelEvent.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoImageRescaleRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoImageRescaleRequest.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoThumbnails.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoThumbnails.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoLengthExtractor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoLengthExtractor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ExtractOneVideoFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ExtractOneVideoFrame.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/enums.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/enums.h"
    )

set(libDB_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryCollection.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/Category.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/Category.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryItem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryPtr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryPtr.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ExifMode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ExifMode.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FastDir.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FastDir.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FileInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/FileInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/GroupCounter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/GroupCounter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDateCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDateCollection.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDB.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDB.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoList.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoPtr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoPtr.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageScout.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageScout.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5Map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5Map.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MediaCount.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MediaCount.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MemberMap.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/MemberMap.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/NewImageFinder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/NewImageFinder.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/OptimizedFileList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/OptimizedFileList.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/RawId.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/RawId.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/TagInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/TagInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/AndCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/AndCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/CategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/CategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ContainerCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ContainerCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ExactCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ExactCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ImageSearchInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ImageSearchInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/NegationCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/NegationCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/NoTagCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/NoTagCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/OrCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/OrCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/SimpleCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/SimpleCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ValueCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/ValueCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/WildcardCategoryMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/search/WildcardCategoryMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/AttributeEscaping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/AttributeEscaping.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/CompressFileInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/CompressFileInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/ElementWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/ElementWriter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/FileReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/FileReader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/FileWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/FileWriter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/NumberedBackup.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/NumberedBackup.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/XmlReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/XML/XmlReader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/GlobalCategorySortOrder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/DB/GlobalCategorySortOrder.h"
    )

set(libImportExport_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Export.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Export.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Import.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Import.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportMatcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportMatcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/XMLHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/XMLHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MiniViewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MiniViewer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportHandler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportHandler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImageRow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImageRow.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportSettings.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportSettings.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/KimFileReader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/KimFileReader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MD5CheckPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MD5CheckPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Logging.h"
    )

set(libAnnotationDialog_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Dialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Dialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListSelect.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListSelect.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreview.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreview.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreviewWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreviewWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/DateEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/DateEdit.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/CompletableLineEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/CompletableLineEdit.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListViewItemHider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListViewItemHider.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShowSelectionOnlyManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShowSelectionOnlyManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShortCutManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShortCutManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ResizableFrame.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ResizableFrame.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/DescriptionEdit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/DescriptionEdit.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/AreaTagSelectDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/AreaTagSelectDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Logging.h"
    )

set(libBrowser_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/OverviewPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/OverviewPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/CategoryPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/CategoryPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/ImageViewPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/ImageViewPage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/TreeFilter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/TreeFilter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/Breadcrumb.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/Breadcrumb.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BreadcrumbList.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/BreadcrumbList.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/AbstractCategoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/AbstractCategoryModel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/FlatCategoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/FlatCategoryModel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/TreeCategoryModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/TreeCategoryModel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/CenteringIconView.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/CenteringIconView.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/Logging.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/enums.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Browser/enums.h"
    )

set(libExif_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/documentation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/InfoDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/InfoDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/TreeView.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/TreeView.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/RangeWidget.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/RangeWidget.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/ReReadDialog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/ReReadDialog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/Grid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/Grid.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialogSettings.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialogSettings.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/MetaDataDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Exif/MetaDataDisplay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ExifPage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Settings/ExifPage.h"
    )

set(libBackgroundTaskManager_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobInterface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobInterface.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/StatusIndicator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/StatusIndicator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobViewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobViewer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobModel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobModel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/JobInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/CompletedJobInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/CompletedJobInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/Priority.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/Priority.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/PriorityQueue.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/PriorityQueue.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/Logging.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundTaskManager/Logging.h"
    )

set(libBackgroundJobs_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/SearchForVideosWithoutLengthInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/SearchForVideosWithoutLengthInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/ReadVideoLengthJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/ReadVideoLengthJob.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/SearchForVideosWithoutVideoThumbnailsJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/SearchForVideosWithoutVideoThumbnailsJob.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/HandleVideoThumbnailRequestJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/HandleVideoThumbnailRequestJob.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/ExtractOneThumbnailJob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/BackgroundJobs/ExtractOneThumbnailJob.h"
    )

option(KPA_ENABLE_REMOTECONTROL "Build with support for companion Android/QML app." OFF)
set(libRemoteControl_SRCS)
set(libRemoteControl_LIBS)
if(KPA_ENABLE_REMOTECONTROL)
    find_package(Qt6 ${MINIMUM_QT6_VERSION} REQUIRED COMPONENTS Network)
    # requires cmake 3.12
    add_compile_definitions(KPA_ENABLE_REMOTECONTROL)
    set(libRemoteControl_LIBS Qt6::Network)
    set(libRemoteControl_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteCommand.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteCommand.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteConnection.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteConnection.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/Server.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/Server.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteInterface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteInterface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/SearchInfo.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/SearchInfo.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteImageRequest.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/RemoteImageRequest.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/ImageNameStore.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/ImageNameStore.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/ConnectionIndicator.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/ConnectionIndicator.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/Logging.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RemoteControl/Logging.h"
        )
endif()

set(libMap_SRCS)
if(Marble_FOUND)
    set(libMap_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser/GeoPositionPage.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Browser/GeoPositionPage.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/enums.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/enums.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/GeoCluster.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/GeoCluster.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/MapView.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/MapView.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/Logging.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/Logging.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/GeoCoordinates.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Map/GeoCoordinates.h"
        )
endif()

add_subdirectory(lib)
add_subdirectory(images)
add_subdirectory(icons)
add_subdirectory(demo)
add_subdirectory(themes)
add_subdirectory(scripts)

add_subdirectory(doc)

########### next target ###############

set(kphotoalbum_SRCS
    main.cpp
    kphotoalbum.qrc
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
target_include_directories(kphotoalbum PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# External components
target_link_libraries(kphotoalbum
    PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Xml
    Qt6::Widgets
    Qt6::DBus
    KPA::Base
    KPA::Thumbnails
    KPA::Exif
    ${JPEG_LIBRARY}
    KF6::Archive
    KF6::Completion
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::IconWidgets
    KF6::JobWidgets
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::TextWidgets
    KF6::XmlGui
    KF6::WidgetsAddons
    KF6::ColorScheme
    ${libRemoteControl_LIBS}
    )

if(KF6Purpose_FOUND)
    target_link_libraries(kphotoalbum PRIVATE KF6::Purpose KF6::PurposeWidgets)
endif()

if(KDcrawQt6_FOUND)
    target_link_libraries(kphotoalbum PRIVATE KDcrawQt6)
endif()

if(Marble_FOUND)
    target_link_libraries(kphotoalbum PRIVATE Marble)
endif()

if(LIBVLC_FOUND)
  include_directories(${LIBVLC_INCLUDE_DIR})
  target_sources(kphotoalbum PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VLCDisplay.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VLCDisplay.h"
  )
  target_link_libraries(kphotoalbum PRIVATE ${LIBVLC_LIBRARY})
endif()

if(Phonon4Qt6_FOUND)
  target_sources(kphotoalbum PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/PhononDisplay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Viewer/PhononDisplay.h"
  )
  target_link_libraries(kphotoalbum PRIVATE Phonon::phonon4qt6)
endif()
install(TARGETS kphotoalbum ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(kpa-thumbnailtool)

########### install files ###############

install(PROGRAMS org.kde.kphotoalbum.desktop org.kde.kphotoalbum-import.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kphotoalbumrc DESTINATION ${KDE_INSTALL_CONFDIR})
install(FILES default-setup DESTINATION "${KDE_INSTALL_DATADIR}/kphotoalbum")
install(FILES org.kde.kphotoalbum.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
ki18n_install(po)

if(KF6DocTools_FOUND)
    kdoctools_install(po)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# vi:expandtab:tabstop=4 shiftwidth=4:
