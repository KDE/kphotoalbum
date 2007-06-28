
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  imagepage.html  kphotoalbum.theme  mainpage.css  mainpage.html download.png DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum/themes/blue )



#original Makefile.am contents follow:

#themesdir = $(kde_datadir)/kphotoalbum/themes/blue
#themes_DATA = imagepage.html  kphotoalbum.theme  mainpage.css  mainpage.html download.png
