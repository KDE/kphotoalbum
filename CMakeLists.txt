
find_package(KDE4 REQUIRED)
include( KDE4Defaults)

ADD_DEFINITIONS(${QT_DEFINITIONS} 
                ${KDE4_DEFINITIONS}
                -DQT3_SUPPORT 
                -DQT3_SUPPORT_WARNINGS
                ${KDE4_ENABLE_EXCEPTIONS}
                -DOVERRIDE=
               )

INCLUDE_DIRECTORIES(${KDE4_INCLUDES}
                    ${QT_INCLUDES}
                   )
#PENDING macro_optional_find_package(Kipi)
macro_optional_find_package(Exiv2)
macro_optional_find_package(JPEG)

include(ConvenienceLibs.cmake)

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
set(libPlugins_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/ImageInfo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/ImageCollection.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Interface.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Plugins/CategoryImageCollection.cpp )
set(libViewer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/Draw.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/CircleDraw.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/RectDraw.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/LineDraw.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/DrawList.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/ViewerWidget.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/Viewer/DrawHandler.cpp 
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
	${CMAKE_CURRENT_SOURCE_DIR}/MainWindow/DirtyIndicator.cpp )

set(libImageManager_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageLoader.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/Manager.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageRequest.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageClient.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/ImageDecoder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/RawImageDecoder.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/ImageManager/parse.c 
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
	${CMAKE_CURRENT_SOURCE_DIR}/ImportExport/MiniViewer.cpp )

set(libAnnotationDialog_SRCS  
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Dialog.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/Editor.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListSelect.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ImagePreview.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/KDateEdit.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/CompletableLineEdit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ListViewItemHider.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/AnnotationDialog/ShowSelectionOnlyManager.cpp )

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

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SQLDB)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KEXI_DIR)")
if(KIPI_FOUND)
	#TODO
endif(KIPI_FOUND)

set(EXIV2_SRCS)
#PENDING if(EXIV2_FOUND)
#PENDING 	set(EXIV2_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/Browser/ExifFolder.cpp)
#PENDING endif(EXIV2_FOUND)

########### next target ###############

set(kphotoalbum_SRCS main.cpp ${libdatebar_SRCS} ${libSettings_SRCS} ${libsurvey_SRCS} ${libxmldb_SRCS} ${libThumbnailView_SRCS} ${libPlugins_SRCS} ${libViewer_SRCS} ${libCategoryListView_SRCS} ${libHTMLGenerator_SRCS} ${libUtilities_SRCS} ${libMainWindow_SRCS} ${libImageManager_SRCS} ${libDB_SRCS} ${libImportExport_SRCS} ${libAnnotationDialog_SRCS} ${EXIV2_SRCS} ${libBrowser_SRCS}) 

kde4_add_executable(kphotoalbum ${kphotoalbum_SRCS})

target_link_libraries(kphotoalbum  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} ${JPEG_LIBRARY} kmediaplayer )

if(EXIV2_FOUND)
 	target_link_libraries(kphotoalbum ${EXIV2_LIBRARIES} )
endif(EXIV2_FOUND)


install(TARGETS kphotoalbum DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  kphotoalbum.desktop kphotoalbum-import.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES  kphotoalbumrc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES  x-vnd.kde.kphotoalbum-import.desktop DESTINATION ${MIME_INSTALL_DIR}/application )
install( FILES  tips DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum )
install( FILES  kphotoalbumui.rc default-setup default-layout.xml exif-sqlite.db DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum )



#original Makefile.am contents follow:

#if compile_sqldb
#  LIBKPHOTOALBUM_SQLDB=SQLDB/libsqldb.la
#  SQLDB=SQLDB
#  LIBKPHOTOALBUM_KEXIDB=kexi/kexidb/libkphotoalbumkexidb.la kexi/kexiutils/libkphotoalbumkexiutils.la kexi/kexidb/parser/libkphotoalbumkexidbparser.la
#  KEXI_DIR=kexi
#endif
#
#if compile_with_kipi
#  KPHOTOALBUM_KIPI_LIBS=-lkipi
#endif
#
#if compile_with_exiv2
#  EXIF_DIR = Exif
#  KPHOTOALBUM_EXIV2_LIBS = Exif/libexif.la -lexiv2
#endif
#
##if compile_with_kexidb
##  KPHOTOALBUM_KEXI_LIBS=-lkexidb -lkexiutils -lkexidbparser
##endif
#
#SUBDIRS = images icons demo themes Survey XMLDB DateBar $(SQLDB) $(EXIF_DIR) ThumbnailView Plugins Viewer Browser ImportExport \
#          AnnotationDialog MainWindow Settings Utilities ImageManager DB $(KEXI_DIR) HTMLGenerator CategoryListView
#
#INCLUDES =  -I$(srcdir)/Survey $(all_includes)
#
#bin_PROGRAMS = kphotoalbum
#kphotoalbum_SOURCES = main.cpp
#kphotoalbum_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kphotoalbum_LDADD =  MainWindow/libMainWindow.la Survey/libsurvey.la ImportExport/libImportExport.la \
#		     XMLDB/libxmldb.la DateBar/libdatebar.la Browser/libBrowser.la \
#		     Plugins/libPlugins.la Viewer/libViewer.la ThumbnailView/libThumbnailView.la \
#		     AnnotationDialog/libAnnotationDialog.la Settings/libSettings.la \
#		     Utilities/libUtilities.la ImageManager/libImageManager.la DB/libDB.la \
#		     CategoryListView/libCategoryListView.la \
#                     $(LIBKPHOTOALBUM_SQLDB) $(LIB_KIO) -ljpeg $(KPHOTOALBUM_KIPI_LIBS) $(KPHOTOALBUM_KEXI_LIBS) \
#                     $(LIBKPHOTOALBUM_KEXIDB) HTMLGenerator/libHTMLGenerator.la\
#	             $(KPHOTOALBUM_EXIV2_LIBS) -lkmediaplayer
#
#METASOURCES = AUTO
#KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 
#
#rcdir = $(kde_datadir)/kphotoalbum
#rc_DATA = kphotoalbumui.rc default-setup default-layout.xml exif-sqlite.db
#
#appsdatadir=$(kde_datadir)/kphotoalbum
#appsdata_DATA=tips
#
#xdg_apps_DATA = kphotoalbum.desktop kphotoalbum-import.desktop
#
#mimetypeapplicationdatadir = $(kde_mimedir)/application
#mimetypeapplicationdata_DATA = x-vnd.kde.kphotoalbum-import.desktop
#
#messages: rc.cpp
#	$(PREPARETIPS) > tips.cc
#	$(XGETTEXT) `find . -name \*.cpp -o -name \*.cc -o -name \*.h | grep -v kexi` -o $(podir)/kphotoalbum.pot
#	rm -f tips.cc 
#
#kde_conf_DATA = kphotoalbumrc
