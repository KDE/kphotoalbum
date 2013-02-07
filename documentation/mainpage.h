/**
   \mainpage
   Welcome to the KPhotoAlbum source code documentation, which is generated with doxygen. To generate your own copy, simply run <tt>doxygen</tt> in the kphotoalbum source directory.

   A few related pages that you should read:
   \li \ref coding-standards
   \li \ref phrase-book
   \li \ref videothumbnails

   KPhotoAlbum is split into a number of modules, each module is a directory of its own on the hard disk and a namespace in the source code.
   The following is a list of modules:

   <h2>Main GUI component</h2>
   \li \ref MainWindow - The main window and associated dialogs.
   \li \ref Browser - This is the browser where you narrow your way to the image you want to see.
   \li \ref ThumbnailView - The thumbnail viewer.
   \li \ref Viewer - The image/video viewer.
   \li \ref AnnotationDialog - This is the dialog where you tag your images (the one you get to using Ctrl+1 and Ctrl+2).
   \li \ref DateBar - The date bar at the bottom of the main screen.

   <h2>Other GUI components</h2>
   \li \ref ImportExport - Import/Export dialog and asscociated classes.
   \li \ref Settings - The Settings dialog and backend classes.
   \li \ref HTMLGenerator - The buildin HTML generator.
   \li \ref CategoryListView - This is the tree view used in the annotation dialog.
   \li \ref Exif - Exif related dialog and backend classes


   <h2>Database backend</h2>
   \li \ref DB The abstract interface for the database backend.
   \li \ref XMLDB The XML based database backend.

   <h2>Backend</h2>
   \li \ref ImageManager - Thumbnail loader
   \li \ref Plugins - KIPI plug-in management
   \li \ref Utilities - Miscellaneous utility classes
   \li \ref BackgroundTaskManager
   \li \ref BackgroundJobs - Jobs for the BackgroundTaskManager
 */
// vi:expandtab:tabstop=4 shiftwidth=4:
