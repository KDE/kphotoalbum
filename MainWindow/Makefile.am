noinst_LTLIBRARIES = libMainWindow.la

INCLUDES = -I$(srcdir)/.. $(all_includes)

libMainWindow_la_SOURCES = DeleteDialog.cpp DeleteThumbnailsDialog.cpp DonateDialog.cpp FeatureDialog.cpp \
                           InvalidDateFinder.cpp TokenEditor.cpp WelcomeDialog.cpp \
			               Window.cpp SplashScreen.cpp ExternalPopup.cpp SearchBar.cpp ImageCounter.cpp DirtyIndicator.cpp


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

if compile_sqldb
  # Kexi header doesn't compile with these macros
  KDE_CXXFLAGS += -UQT_NO_CAST_ASCII -UQT_CAST_NO_ASCII
endif

METASOURCES = AUTO
