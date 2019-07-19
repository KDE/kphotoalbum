#!/bin/bash
# Apply clang-format in-place to all source files
# Note: AndroidRemoteControl is excluded for now:

clang-format -i \
	DB/*.{cpp,h} \
	Utilities/*.{cpp,h} \
	ThumbnailView/*.{cpp,h} \
	AnnotationDialog/*.{cpp,h} \
	BackgroundJobs/*.{cpp,h} \
	BackgroundTaskManager/*.{cpp,h} \
	Browser/*.{cpp,h} \
	CategoryListView/*.{cpp,h} \
	DateBar/*.{cpp,h} \
	Exif/*.{cpp,h} \
	HTMLGenerator/*.{cpp,h} \
	ImageManager/*.{cpp,h} \
	ImportExport/*.{cpp,h} \
	MainWindow/*.{cpp,h} \
	MainWindow/DuplicateMerger/*.{cpp,h} \
	Map/*.{cpp,h} \
	Plugins/*.{cpp,h} \
	RemoteControl/*.{cpp,h} \
	Settings/*.{cpp,h} \
	Viewer/*.{cpp,h} \
	XMLDB/*.{cpp,h}
