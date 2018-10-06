# Pre release tests for KPhotoAlbum

This is a list of things to check before putting out a new version of kphotoalbum.

## Notes

 - Feel free to add new tests!
 - When writing helper snippets, assume the kphotoalbum source directory as current working directory.
 - If not otherwise specified, the tests assume the default configuration options.
   Either start them in a clean environment, or set XDG_CONFIG_HOME so that an existing kphotoalbumrc is not used.

## Compatibility checks

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

## Semi-automated checks

Some integration-tests can be run by using the script `testcases/integration-tests.sh`.
Run with parameter `--all` to execute all tests, or try its help for more info.
