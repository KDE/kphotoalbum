//krazy:skip
/**
  \namespace Settings
  \brief Classes for storing and configuring the settings of KPhotoAlbum

  <h2>The GUI part</h2>
  The class \ref SettingsDialog is the topmost class for the GUI
  components. Each page in there are represented by the classes \ref
  GeneralPage, \ref ThumbnailsPage, \ref ViewerPage, \ref CategoryPage, \ref SubCategoriesPage, \ref PluginsPage,
  \ref ExifPage, and \ref DatabaseBackendPage.

  The class \ref ViewerSizeConfig is a minor utility class used on the \ref
  ViewerPage.


  \ref CategoryItem represent the items on the \ref CategoryPage, plus some
  of the methods for adding and removing categories.

  <h2>The backend</h2>
  All settings on the configuration pages are stored in SettingsData, which
  is the interface the rest of the application uses.
**/
// vi:expandtab:tabstop=4 shiftwidth=4:
