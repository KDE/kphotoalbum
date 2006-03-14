noinst_LTLIBRARIES = \
    libsurvey.la

INCLUDES = \
    $(all_includes) -I$(srcdir)/..

libsurvey_la_SOURCES = \
	Question.cpp SurveyDialog.cpp TextQuestion.cpp\
	InfoPage.cpp AlternativeQuestion.cpp MySurvey.cpp SurveyCount.cpp

METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII 

icondir = $(kde_datadir)/kphotoalbum/pics
icon_DATA = questionmark.png questionmark-flipped.png exclamationmark.png
