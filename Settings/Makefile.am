noinst_LTLIBRARIES = libSettings.la

if compile_sqldb
INCLUDES = -I$(srcdir)/../kexi -I$(srcdir)/.. $(all_includes)
else
INCLUDES = -I$(srcdir)/.. $(all_includes)
endif

libSettings_la_SOURCES = SettingsData.cpp  SettingsDialog.cpp  ViewerSizeConfig.cpp CategoryItem.cpp


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
