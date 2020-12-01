// SPDX-FileCopyrightText: 2009 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

//krazy:skip
/**
  \namespace MainWindow
  \brief The main window plus a number of dialogs.

  <h2>The Main %Window</h2>
  The class \ref Window is the central collection point for most of the
  application. It is responsible for setting up the main window, and
  connecting all its sub components.

  This is also the place where you will find the code setting up the menu
  bars and tool bars, and connecting their signals to slots

  <h2>Main %Window Components</h2>
  This namespace also contains a few classes that you will find in the main
  %window itself. These are:

  \li SearchBar - This is the search bar located in the toolbar.
  \li DirtyIndicator - this is the small image in the toolbar that indicate
  whether there are unsaved data. At the same time it is also the authority
  regarding whether there are saved data (so this is the place where there
  is a boolean the rest of the application will set when there are unsaved changes).
  \li ImageCounter  - this is the label in the status bar showing amount of
  matched and total images.
  \li BreadcrumbViewer - this is the widget containing the breadcrumbs
  located in the status bar.


  <h2>Dialogs</h2>

  In addition to the main %window and its component, this namespace also
  contains a number of dialogs, that are accessed from the menu bar.

  \li DeleteDialog - the Delete Images/Videos dialog.
  \li FeatureDialog - The Feature Status dialog.
  \li StatisticsDialog - The statistics dialog.
  \li TokenEditor - The dialog used for deleting tokens.
  \li WelcomeDialog - The dialog shown the first time a user starts KPhotoAlbum.
  \li InvalidDateFinder - The dialog used to find images with invalid or
  incomplete dates.

  <h2>Other Classes</h2>
  \li ExternalPopup - this is the sub menu of the context menu, showing the
  external application that the given image can be handed to.
  \li SplashScreen - the splash screen shown at start up


**/
// vi:expandtab:tabstop=4 shiftwidth=4:
