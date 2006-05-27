noinst_LTLIBRARIES = libDialogs.la

INCLUDES = -I$(srcdir)/.. $(all_includes)

libDialogs_la_SOURCES = DeleteDialog.cpp DeleteThumbnailsDialog.cpp DonateDialog.cpp FeatureDialog.cpp \
                        HtmlExportDialog.cpp InvalidDateFinder.cpp TokenEditor.cpp WelcomeDialog.cpp


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
