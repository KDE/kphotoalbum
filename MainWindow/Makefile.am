noinst_LTLIBRARIES = libMainWindow.la

if compile_sqldb
INCLUDES = -I$(srcdir)/../kexi -I$(srcdir)/.. $(all_includes)
else
INCLUDES = -I$(srcdir)/.. $(all_includes)
endif

libMainWindow_la_SOURCES = DeleteDialog.cpp DeleteThumbnailsDialog.cpp DonateDialog.cpp FeatureDialog.cpp \
                        InvalidDateFinder.cpp TokenEditor.cpp WelcomeDialog.cpp \
			Window.cpp SplashScreen.cpp ExternalPopup.cpp SearchBar.cpp ImageCounter.cpp 


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
