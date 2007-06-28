
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  multiconfig.jpg search.jpg splash.png splash-large.png movie-clip.png DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum/pics )



#original Makefile.am contents follow:

#picsdir = $(kde_datadir)/kphotoalbum/pics
#pics_DATA = multiconfig.jpg search.jpg splash.png splash-large.png movie-clip.png
#
