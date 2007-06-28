
find_package(KDE4 REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

macro_optional_find_package(Kipi)
macro_optional_find_package(Exiv2)

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

add_subdirectory( images )
add_subdirectory( icons )
add_subdirectory( demo )
add_subdirectory( themes )
add_subdirectory( Survey )
add_subdirectory( Browser )
add_subdirectory( ImportExport )
add_subdirectory( AnnotationDialog )
add_subdirectory( MainWindow )
add_subdirectory( ImageManager )
add_subdirectory( DB )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(SQLDB)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(EXIF_DIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KEXI_DIR)")
if(KIPI_FOUND)
	#TODO
endif(KIPI_FOUND)


########### next target ###############

set(kphotoalbum_SRCS main.cpp ${libdatebar_SRCS} ${libSettings_SRCS} ${libsurvey_SRCS} ${libxmldb_SRCS} ${libThumbnailView_SRCS} ${libPlugins_SRCS} ${libViewer_SRCS} ${libCategoryListView_SRCS} ${libHTMLGenerator_SRCS} ${libUtilities_SRCS}) 

kde4_automoc(${kphotoalbum_SRCS})

kde4_add_executable(kphotoalbum ${kphotoalbum_SRCS})

target_link_libraries(kphotoalbum  ${KDE4_KDECORE_LIBS} MainWindow ImportExport Browser AnnotationDialog Utilities ImageManager DB  ${KDE4_KIO_LIBS} jpeg kmediaplayer )

install(TARGETS kphotoalbum DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES  kphotoalbum.desktop kphotoalbum-import.desktop DESTINATION ${XDG_APPS_DIR} )
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
