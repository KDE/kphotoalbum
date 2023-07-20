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

### Changed

### Dependencies

### Deprecated

### Fixed
 - Fix issue with KPhotoAlbum not asking to save before exit (#472427)

### Removed

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
