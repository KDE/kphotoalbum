noinst_LTLIBRARIES = libAnnotationDialog.la

INCLUDES =  -I$(srcdir)/.. $(all_includes)

libAnnotationDialog_la_SOURCES = Dialog.cpp Editor.cpp ListSelect.cpp ImagePreview.cpp KDateEdit.cpp CompletableLineEdit.cpp \
				 ListViewItemHider.cpp ShowSelectionOnlyManager.cpp

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_THREADS) -DQT_NO_CAST_ASCII -DQT_CAST_NO_ASCII

METASOURCES = AUTO
