
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/kphotoalbum/icons )

kde4_install_icons( ${ICON_INSTALL_DIR} )



#original Makefile.am contents follow:

#appicondir = $(kde_datadir)/kphotoalbum/icons
#appicon_ICON = ellipsetool linetool recttool selecttool texttool key video
#
#KDE_ICON = kphotoalbum
