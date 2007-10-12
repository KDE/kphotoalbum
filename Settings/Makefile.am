noinst_LTLIBRARIES = libSettings.la

INCLUDES = -I$(srcdir)/.. $(all_includes)

libSettings_la_SOURCES = SettingsData.cpp  SettingsDialog.cpp  ViewerSizeConfig.cpp CategoryItem.cpp


KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

if compile_sqldb
  # Kexi header doesn't compile with these macros
  KDE_CXXFLAGS += -UQT_NO_CAST_ASCII -UQT_CAST_NO_ASCII
endif

METASOURCES = AUTO
