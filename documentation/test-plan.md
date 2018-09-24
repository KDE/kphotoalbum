# Pre release tests for KPhotoAlbum

This is a list of things to check before putting out a new version of kphotoalbum.

## Notes

 - Feel free to add new tests!
 - When writing helper snippets, assume the kphotoalbum source directory as current working directory.

## Compatibility

### Convert from older database formats

For some context, also read the file `testcases/db/version6-transition/README`…

1. Run script `testcases/db/version6-transition/check.sh`
2. When kphotoalbum is started, save the database and quit
3. If the results did match: you're fine!
4. If the results did not match:
    1. Was there a change in the db format?
    2. Are all other changes reasonable and expected?
    3. If yes: update the result.xml files.
      If not: start debugging :|

Helper snippet:
````
testcases/db/version6-transition/check.sh
````


## Feature tests

### Mark as untagged

1. Load demo database
2. Make sure that "mark as untagged" is configured
3. Copy image file to demo database
4. Check if new image has the *untagged* tag set.

Helper snippet:
````
check_untagged() {
    DEMODB=/tmp/kphotoalbum-demo-$USER
    kphotoalbum --demo &
    while ! test -d "$DEMODB" ; do sleep 0.5 ;done
    convert -size 700x460  magick:rose "$DEMODB/rose.jpg"
}
check_untagged
````

### Auto-stacking
