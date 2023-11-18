<!--
SPDX-FileCopyrightText: 2022-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
SPDX-FileCopyrightText: 2022-2023 Tobias Leupold <tl at stonemx dot de>

SPDX-License-Identifier: CC-BY-SA-4.0

Template to use after a release:

(Unreleased)
------------

### Added

### Changed

### Dependencies

### Deprecated

### Fixed

### Removed

### Security

-->
Changelog
=========

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

The change log for older releases (before 5.9.0) can be found in CHANGELOG.old.

(Unreleased)
------------

### Added
 - Support annotating images from the viewer by using letters to assign tags.
   Use the context menu and select "Annotate | Assign Tags" to enable.
   More information is available in the KPhotoAlbum handbook.
 - Add option to sort category page by natural order (feature #475339).
   Natural sort order takes the locale into account and sorts numeric values properly (e.g. sort "9" before "10").
 - Allow selecting a date range in the DateBar via keyboard (Use "Shift + Left|Right")
 - Allow closing the annotation dialog's fullscreen preview using the Escape key.
 
### Changed
 - In the viewer window, using the letters A-Z to assign tokens now needs to be explicitly enabled.
   You can do this by opening the context menu and selecting "Annotate | Assign Tokens".
 - When KPhotoAlbum is started in demo mode and a previously saved demo database exists, the old demo database is no longer overwritten.
 - The ui.rc file (kphotoalbumui.rc) is now deployed as a Qt resource instead of an on-disk file.
 - Improved usability of "Invoke external program" menu (#474819)
 - No longer set the default shortcut for "Use current video frame in thumbnail view" to Ctrl+S and avoid shortcut conflict.
 - Restrict context menu entries for fullscreen preview of annotation dialog to a sane set of actions.

### Dependencies
 - Add support for exiv2 0.28.1
 - Remove support for exiv2 < 0.27

### Deprecated

### Fixed
 - Fix issue with KPhotoAlbum not asking to save before exit (#472427)
 - Fix crash when right-clicking in the empty space of the tag lists of the annotation dialog (#472523)
 - Prevent showing selected thumbnails only if no thumbnails are selected, so that no crash can occur when showing the context again afterwards (#473324)
 - Fix crash when switching from video to image (#473587)
 - Fix program freeze when the viewer window is closed while playing a video using the VLC backend.
 - Fix crash when both the annotation dialog and the viewer window is open and the user right-clicks on the viewer window (#473762)
 - Fix crash when annotations are not saved and then the user right-clicks on the viewer window (#474151)
 - Fix crash when user opens the last image/video in viewer then deletes the image and then accesses the viewer context menu (#474392)
 - Fix several crashes when deleting an image/video that is currently being annotated in the annotation dialog (#475387, #475388)
 - Correctly discard images from annotation dialog if they are deleted elsewhere (e.g. in the thumbnail view).
 - Fix failed assertion and potential database corruption when searching for new image while the new image search is already running (#475529)
 - Fix crash when trying to copy or link an image from the annotation dialog's fullscreen preview (#475585)
 - Use consistent icon sizes in category browser ("Tree" and "Tree with User Icons")
 - Fix crash when sorting selected images while thumbnail display order is "Newest First" (#476651)
 - Fix invalid assertion when date bar selection is extended beyond the valid range (#476862)
 - Fix failed assertion when creating a tag group by drag and drop in a category that does not yet have any tag groups (#477195)

### Removed
 - It is no longer possible to annotate images from the viewer by pressing "/" and typing tag names.
 - It is no longer possible to change an image through the annotation dialog's fullscreen image preview.

### Security

KPhotoAlbum 5.11.0 (2023-07-12)
-------------------------------

### Changed
 - "Recalculate Checksums" in the Maintenance menu and "Refresh Selected Thumbnails" in the thumbnail context menu have been unified to do exactly the same.
   Both actions have been renamed to "Refresh Selected Thumbnails and Checksums".
 - Simplified logging categories: "kphotoalbum.XMLDB" was merged into "kphotoalbum.DB"

### Dependencies
 - KPhotoAlbum can now be compiled using exiv2 0.28.

### Fixed
 - Fix issue where non-empty time units in the date bar were incorrectly greyed out (#467903)
 - Fix bug with the date bar showing and selecting incorrect date ranges (#468045)
 - Fix crash when the annotation dialog is opened from the viewer window and the viewer is closed before the annotation dialog (#470889)
 - Fix inconsistent UI where menu actions would not immediately be updated to reflect a change (#472109, #472113)

KPhotoAlbum 5.10.0 (2023-03-25)
-------------------------------

### Added
 - Allow setting keyboard shortcuts for Date Bar component.
 - Visually differentiate the occupied date range in the date bar by graying out the unoccupied edge areas.

### Changed
 - View-related actions formerly found in the "Settings" menu were moved to the "View" menu.
 - Make options "Display Labels in Thumbnail View" and "Display Categories in Thumbnail View" reachable via the "View" menu
   and allow both actions to be assigned keyboard shortcuts (Implements: #145346).
 - Store the untagged tag information inside the index.xml file instead of the Settings (Implements: #461206).
 - Change scroll direction in the annotation dialog's date edit fields to match common (western) expectations and the date picker.
 - Prevent scrolling past the occupied areas of the date bar.
 - Files are now always created with group read/write permissions (Fixes: #438128).
 - When exiting the demo mode, the demo database is now always saved if it isn't deleted.

### Dependencies
 - CMake: 3.18
 - Qt5: 5.15
 - KDE Frameworks: 5.78

### Fixed
 - Improve readability of "Show Tooltips in Thumbnails Window" tooltip.
 - Fix image selection order for newly added images (Fixes: 442325).
 - Improve date bar behavior when zooming the date bar and changing views (Fixes: 357237).

### Removed
 - Default shortcut for "View" images was removed.<br>
   Pressing "Enter" to open the viewer is now the preferred way.
   To restore the old behavior, reassign the shortcut via "Settings | Configure Keyboard Shortcuts...".

KPhotoAlbum 5.9.1 (2022-09-05)
------------------------------

### Fixed
- Due to a problem with the tarme.rb releasme script, the 5.9.0 tarball lacked all localisation
  data. We thus had to withdraw the release and tag a new one.


KPhotoAlbum 5.9.0 (2022-09-04)
------------------------------

### Added
- Generic file metadata (size, last changed date etc.) can now be viewed via the Exif
  metadata dialog (Implements: 443552).
- Support other video backends (libVLC, QtAV) in addition to Phonon.
- Add volume controls to video player.

### Fixed
- Fix crash when forgetting to select images upon import (Fixes: 445404)
- Fix faulty assertion when video thumbnail files cannot be written (Fixes: 446258)
- Remove incomplete URL encoding of non-ASCII characters in HTML export (Fixes: 452458)
- Fix crash when reimporting deleted files from a .kim file (Fixes: 444792)
- Fix multiple issues identified by code analysis tools.

### Removed
- Tip of the day feature was removed because it is no longer supported by KDE Frameworks.
