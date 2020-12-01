<!--
SPDX-License-Identifier: CC-BY-SA-4.0
SPDX-FileCopyrightText: 2018-2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
-->

# Pre release tests for KPhotoAlbum

This is a list of things to check before putting out a new version of kphotoalbum.

## Notes

 - Feel free to add new tests!
 - When writing helper snippets, assume the kphotoalbum source directory as current working directory.
 - If not otherwise specified, the tests assume the default configuration options.
   Either start them in a clean environment, or set XDG_CONFIG_HOME so that an existing kphotoalbumrc is not used.

## Semi-automated checks

Some integration-tests can be run by using the script `testcases/integration-tests.sh`.
Run with parameter `--all` to execute all tests, or try its help for more info.

## Manual tests

At least start KPhotoAlbum in the demo mode and try the following things:

### Annotate images

Try annotating some image, see if something is not behaving as it should.

 - Annotate individual items and multiple items at a time
 - Try the fullscreen preview (Ctrl+Space)
 - Tag several people and add positionable tags
 - Revert changes on an item
 - Copy tags from the previously tagged image
 - Delete an image
 - Change the date/time or set a fuzzy date
 - Set a rating

### Thumbnail view

 - Create a stack, remove images from a stack, add images to a stack.
 - Set/unset some tokens (a-z)
 - Set some ratings (1-5)
 - Try the quick-filter for ratings (use the widget) and for tokens (Alt+Shift+a-z)
 - Play around with the date bar (chronological histogram widget
 - Try scaling the thumbnails)
 - Rebuild some or all thumbnails

### Image Viewer

 - Try it
 - Does it play videos correctly?

### Maintenance Menu

 - Play around with the menu items...
