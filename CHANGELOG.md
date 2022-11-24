<!--
SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
SPDX-FileCopyrightText: 2022 Tobias Leupold <tl@stonemx.de>

SPDX-License-Identifier: CC-BY-SA-4.0
-->
Changelog
=========

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

The change log for older releases (before 5.9.0) can be found in CHANGELOG.old.

## [UNRELEASED]

### Added
 - Allow setting keyboard shortcuts for Date Bar component.

### Changed
 - View-related actions formerly found in the "Settings" menu were moved to the "View" menu.
 - Make options "Display Labels in Thumbnail View" and "Display Categories in Thumbnail View" reachable via the "View" menu
   and allow both actions to be assigned keyboard shortcuts (Implements: #145346).
 - Store the untagged tag information inside the index.xml file instead of the Settings (Implements: #461206)

### Dependencies
 - CMake: 3.18
 - Qt5: 5.15
 - KDE Frameworks: 5.78

### Deprecated

### Fixed
 - Improve readability of "Show Tooltips in Thumbnails Window" tooltip.

### Removed
 - Default shortcut for "View" images was removed.</br>
   Pressing "Enter" to open the viewer is now the preferred way.
   To restore the old behavior, reassign the shortcut via "Settings | Configure Keyboard Shortcuts..."

### Security

[5.9.1] - 2022-09-05
--------------------

### Fixed
- Due to a problem with the tarme.rb releasme script, the 5.9.0 tarball lacked all localisation
  data. We thus had to withdraw the release and tag a new one.


[5.9.0] - 2022-09-04
--------------------

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
