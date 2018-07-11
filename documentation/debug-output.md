# Getting KPhotoAlbum to print debug output

KPhotoAlbum uses categorized logging.  By default, only warning messages but no
debug messages are emitted.  To enable addditional output, you can either
enable all debug output (```kphotoalbum*.debug=true```) or have more
fine-grained control by using one or more logging categories listed below.

## Enable logging using a configuration file

Create or edit the file ```QtProject/qtlogging.ini``` in the generic config location (usually ```~/.config```) containing the following:

````
    [Rules]
    kphotoalbum.*.debug=true
````

## Enable logging using an environment variable

You can set the environment variable ```QT_LOGGING_RULES``` using the same
syntax as in the configuration file.  Rules are divided by semicolons.

E.g. you can start KPhotoAlbum like this on the commandline:
````
    export QT_LOGGING_RULES='kphotoalbum.*.debug=true'
    kphotoalbum
````

## Logging Categories in KPhotoAlbum

 - ```kphotoalbum.AnnotationDialog```
 - ```kphotoalbum.BackgroundJobs```
 - ```kphotoalbum.BackgroundTaskManager```
 - ```kphotoalbum.Browser```
 - ```kphotoalbum.CategoryListView```
 - ```kphotoalbum.DateBar```
 - ```kphotoalbum.DB```
 - ```kphotoalbum.DB.CategoryMatcher```
 - ```kphotoalbum.DB.ImageScout```
 - ```kphotoalbum.Exif```
 - ```kphotoalbum.FastDir```
 - ```kphotoalbum.HTMLGenerator```
 - ```kphotoalbum.ImageManager```
 - ```kphotoalbum.ImportExport```
 - ```kphotoalbum.MainWindow```
 - ```kphotoalbum.Map```
 - ```kphotoalbum.Plugins```
 - ```kphotoalbum.RemoteControl```
 - ```kphotoalbum.Settings```
 - ```kphotoalbum.ThumbnailView```
 - ```kphotoalbum.timingInformation```
 - ```kphotoalbum.Utilities```
 - ```kphotoalbum.Viewer```
 - ```kphotoalbum.XMLDB```

## Further reading

 - https://doc.qt.io/qt-5/qloggingcategory.html#details
