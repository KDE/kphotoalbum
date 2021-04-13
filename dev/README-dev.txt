Development tools to support KPhotoAlbum development
====================================================

dev/documentation
-----------------

This directory contains developer-focused documentation such as information about coding style and the database file format.
You can also find some notes about debugging.


dev/githooks
------------

You can enable these hooks using the following command:

````
	git config core.hooksPath ./dev/githooks/
````

Alternatively, you can copy the hooks into `.git/hooks` and manually keep them
updated if needed.

About the hooks:
 - 01-pre-commit: this is the default pre-commit hook supplied with git
 - 02-check-copyright-header will remind you to update copyright headers
 - 03-check-untracked_files will try to warn you about files that you forgot to add to git
 - 04-check-clang-format will complain if your commit isn't formatted properly

dev/scripts
-----------

This directory contains scripts useful for developing kphotoalbum.

Scripts that are also useful to power users should usually go to the top level
scripts directory (e.g. kpa-backup.sh).



Additional resources
--------------------

 - https://techbase.kde.org/Development/Git/Configuration
