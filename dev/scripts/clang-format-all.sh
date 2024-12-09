#!/bin/bash
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: CC0-1.0
# Apply clang-format in-place to all source files
# Note: AndroidRemoteControl is excluded for now because clang-format would butcher the symlinks.

clang-format -style=file -i \
	AnnotationDialog/*.{cpp,h} \
	BackgroundJobs/*.{cpp,h} \
	BackgroundTaskManager/*.{cpp,h} \
	Browser/*.{cpp,h} \
	CategoryListView/*.{cpp,h} \
	DateBar/*.{cpp,h} \
	DB/*.{cpp,h} \
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
	ThumbnailView/*.{cpp,h} \
	Utilities/*.{cpp,h} \
	Viewer/*.{cpp,h} \
	lib/*/*.{cpp,h} \
	main.cpp
