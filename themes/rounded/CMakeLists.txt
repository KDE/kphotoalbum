
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  kphotoalbum.theme  mainpage.css  mainpage.html  imagepage.html  		downbar.png   downright.png  rightbar.png  upleft.png  		downleft.png  leftbar.png    upbar.png     upright.png download.png DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum/themes/rounded )



#original Makefile.am contents follow:

#themesdir = $(kde_datadir)/kphotoalbum/themes/rounded
#themes_DATA =  kphotoalbum.theme  mainpage.css  mainpage.html  imagepage.html \
#		downbar.png   downright.png  rightbar.png  upleft.png \
#		downleft.png  leftbar.png    upbar.png     upright.png download.png
#
