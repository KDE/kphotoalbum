Development tools to support KPhotoAlbum development
====================================================


dev/scripts
-----------

This directory contains scripts useful for developing kphotoalbum.

Scripts that are also useful to power users should usually go to the top level
scripts directory (e.g. kpa-backup.sh).


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
