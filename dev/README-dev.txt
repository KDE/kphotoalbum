Development tools to support KPhotoAlbum development
====================================================

dev/documentation
-----------------

This directory contains developer-focused documentation such as information about coding style and the database file format.
You can also find some notes about debugging.


dev/githooks
------------

Since extra-cmake-modules version 5.108 and newer, you don't have to do anything.
CMake will install the hooks for you. See https://api.kde.org/ecm/kde-module/KDEGitCommitHooks.html for details.

About the hooks:
 - default-pre-commit: this is the default pre-commit hook supplied with git
 - check-copyright-header will remind you to update copyright headers on changed files
 - check-untracked-files will try to warn you about files that you forgot to add to git
 - check-cmakelint will run cmakelint on all CMake files that were changed

dev/scripts
-----------

This directory contains scripts useful for developing kphotoalbum.

Scripts that are also useful to power users should usually go to the top level
scripts directory (e.g. kpa-backup.sh).



Additional resources
--------------------

 - https://techbase.kde.org/Development/Git/Configuration
