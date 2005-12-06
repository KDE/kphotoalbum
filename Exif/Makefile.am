noinst_LTLIBRARIES = \
    libexif.la

INCLUDES = \
	 -I$(srcdir)/.. $(all_includes)

libexif_la_SOURCES = Database.cpp  InfoDialog.cpp  SearchDialog.cpp  SearchInfo.cpp TreeView.cpp Info.cpp \
		     SearchDialogSettings.cpp RangeWidget.cpp DatabaseElement.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII $(KIMDABA_KIPI_CXXFLAGS) $(KIMDABA_EXIV2_CXXFLAGS)

METASOURCES = AUTO
