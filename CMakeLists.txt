project(kphotoalbum)

find_package(KDE4 REQUIRED)
include( KDE4Defaults)

# This seems to be needed for KPA to compile with KDE 4.0.0
set(INSTALL_TARGETS_DEFAULT_ARGS RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                 LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                                 ARCHIVE DESTINATION "${LIB_INSTALL_DIR}" COMPONENT Devel )

ADD_DEFINITIONS(${QT_DEFINITIONS} 
                ${KDE4_DEFINITIONS}
                -DQT3_SUPPORT 
                -DQT3_SUPPORT_WARNINGS
                ${KDE4_ENABLE_EXCEPTIONS}
                -DOVERRIDE=
#                -DQT_NO_CAST_FROM_ASCII
#                -DQT_NO_CAST_TO_ASCII
               )

INCLUDE_DIRECTORIES(${KDE4_INCLUDES}
                    ${QT_INCLUDES}
                   )

find_package(JPEG REQUIRED)
if (JPEG_FOUND)
  INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
endif (JPEG_FOUND)

macro_optional_find_package(Kipi)
macro_bool_to_01(KIPI_FOUND HASKIPI)
if(KIPI_FOUND)
  ADD_DEFINITIONS(${KIPI_DEFINITIONS})
  INCLUDE_DIRECTORIES(BEFORE ${KIPI_INCLUDE_DIR})
endif(KIPI_FOUND)

macro_optional_find_package(Kdcraw)
macro_bool_to_01(KDCRAW_FOUND HAVE_KDCRAW)
if(KDCRAW_FOUND)
  ADD_DEFINITIONS(${KDCRAW_DEFINITIONS})
  INCLUDE_DIRECTORIES(BEFORE ${KDCRAW_INCLUDE_DIR})
endif(KDCRAW_FOUND)

macro_optional_find_package(Exiv2)
macro_bool_to_01(EXIV2_FOUND HAVE_EXIV2)

if(QT_QTSQL_FOUND)
  set(SQLDB_SUPPORT TRUE)
endif(QT_QTSQL_FOUND)

# For config-kpa-*.h
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


set(libdatebar_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/DateBar/DateBarWidget.cpp ${CMAKE_CURRENT_SOURCE_DIR}/DateBar/ViewHandler.cpp ${CMAKE_CURRENT_SOURCE_DIR}/DateBar/MouseHandler.cpp)

set(libSettings_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsData.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/Settings/SettingsDialog.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/Settings/ViewerSizeConfig.cpp ${CMAKE_CURRENT_SOURCE_DIR}/Settings/CategoryItem.cpp)

set(libsurvey_SRCS
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/Question.cpp 
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/SurveyDialog.cpp 
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/TextQuestion.cpp
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/InfoPage.cpp 
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/AlternativeQuestion.cpp 
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/MySurvey.cpp 
       ${CMAKE_CURRENT_SOURCE_DIR}/Survey/SurveyCount.cpp)


set(libxmldb_SRCS
  	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/Database.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XMLCategoryCollection.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XMLCategory.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/XMLImageDateCollection.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/NumberedBackup.cpp
  	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/FileReader.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/XMLDB/FileWriter.cpp )

set(libThumbnailView_SRCS 
        ${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailRequest.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailToolTip.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailWidget.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/GridResizeInteraction.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/SelectionInteraction.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/MouseTrackingInteraction.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/ThumbnailView/ThumbnailBuilder.cpp)
set(libPlugins_SRCS)

if(KIPI_FOUND)
set(libPlugins_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Interface.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/ImageCollection.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/ImageInfo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/CategoryImageCollection.cpp)
endif(KIPI_FOUND)

set(libViewer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewerWidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ImageDisplay.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewHandler.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/SpeedDisplay.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ShowOptionAction.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/InfoBox.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CategoryImageConfig.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Display.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/VideoDisplay.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/TextDisplay.cpp)

set(libCategoryListView_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragableListView.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragObject.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/CheckDropItem.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/CategoryListView/DragItemInfo.cpp )

set(libHTMLGenerator_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/HTMLDialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Generator.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/HTMLGenerator/Setup.cpp)
set(libUtilities_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/Utilities/ShowBusyCursor.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Utilities/List.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Set.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Graph.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Utilities/Util.cpp)

set(libMainWindow_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DeleteDialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/FeatureDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/InvalidDateFinder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/TokenEditor.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/WelcomeDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/Window.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SplashScreen.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ExternalPopup.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/SearchBar.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/ImageCounter.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DirtyIndicator.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/StatisticsDialog.cpp )

set(libImageManager_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageLoader.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/Manager.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageRequest.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageClient.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageDecoder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RawImageDecoder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/VideoManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RequestQueue.cpp )

set(libDB_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/Category.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryCollection.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDate.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/MD5Map.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/MemberMap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageInfoList.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageDB.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/FileInfo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/NewImageFinder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/GroupCounter.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryMatcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/DB/ImageSearchInfo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/DB/CategoryItem.cpp )

set(libImportExport_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Export.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/Import.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportMatcher.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/XMLHandler.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MiniViewer.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportHandler.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImageRow.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportDialog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/ImportSettings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/KimFileReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MD5CheckPage.cpp
)

set(libAnnotationDialog_SRCS  
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Dialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListSelect.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreview.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/KDateEdit.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/CompletableLineEdit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListViewItemHider.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShowSelectionOnlyManager.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShortCutManager.cpp 
)

set(libBrowser_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserWidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/ContentFolder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/Folder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/SearchFolder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/BrowserItemFactory.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/ImageFolder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/TypeFolder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/TypeFolderAction.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Browser/ContentFolderAction.cpp
	)


set(libexif_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/Database.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/InfoDialog.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchDialog.cpp  
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/SearchInfo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/TreeView.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/Info.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/RangeWidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/DatabaseElement.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Exif/ReReadDialog.cpp )

add_subdirectory( images )
add_subdirectory( Survey )
add_subdirectory( icons )
add_subdirectory( demo )
add_subdirectory( themes )
if(SQLDB_SUPPORT)
  add_subdirectory( SQLDB )
endif(SQLDB_SUPPORT)

if(KIPI_FOUND)
	#TODO
endif(KIPI_FOUND)

set(EXIV2_SRCS)
if(EXIV2_FOUND)
 	set(EXIV2_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/Browser/ExifFolder.cpp ${libexif_SRCS})
endif(EXIV2_FOUND)

########### next target ###############

set(kphotoalbum_SRCS main.cpp ${libdatebar_SRCS} ${libSettings_SRCS} ${libsurvey_SRCS} ${libxmldb_SRCS} ${libThumbnailView_SRCS} ${libPlugins_SRCS} ${libViewer_SRCS} ${libCategoryListView_SRCS} ${libHTMLGenerator_SRCS} ${libUtilities_SRCS} ${libMainWindow_SRCS} ${libImageManager_SRCS} ${libDB_SRCS} ${libImportExport_SRCS} ${libAnnotationDialog_SRCS} ${EXIV2_SRCS} ${libBrowser_SRCS}) 

kde4_add_executable(kphotoalbum ${kphotoalbum_SRCS})

target_link_libraries(kphotoalbum  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} ${JPEG_LIBRARY} kmediaplayer ${KDE4_PHONON_LIBS})

if(KIPI_FOUND)
  target_link_libraries(kphotoalbum ${KIPI_LIBRARIES})
endif(KIPI_FOUND)

if(KDCRAW_FOUND)
  target_link_libraries(kphotoalbum ${KDCRAW_LIBRARIES})
endif(KDCRAW_FOUND)

if(EXIV2_FOUND)
 	target_link_libraries(kphotoalbum ${EXIV2_LIBRARIES} )
endif(EXIV2_FOUND)

if(SQLDB_SUPPORT)
  target_link_libraries(kphotoalbum SQLDB)
endif(SQLDB_SUPPORT)

install(TARGETS kphotoalbum ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES  kphotoalbum.desktop kphotoalbum-import.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES  kphotoalbumrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES  tips DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum )
install( FILES  kphotoalbumui.rc default-setup default-layout.xml DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum )


configure_file(config-kpa-kdcraw.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpa-kdcraw.h)
configure_file(config-kpa-exiv2.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpa-exiv2.h)
configure_file(config-kpa-kipi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpa-kipi.h)
configure_file(config-kpa-sqldb.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kpa-sqldb.h)

