// SPDX-FileCopyrightText: 2009 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

//krazy:skip
/**
  \namespace Settings
  \brief Classes for storing and configuring the settings of KPhotoAlbum

  <h2>The GUI part</h2>
  The class \ref SettingsDialog is the topmost class for the GUI
  components. Each page in there are represented by the classes
  \ref BirthdayPage,
  \ref CategoryPage,
  \ref DatabaseBackendPage,
  \ref ExifPage,
  \ref FileVersionDetectionPage,
  \ref GeneralPage,
  \ref PluginsPage,
  \ref TagGroupsPage,
  \ref ThumbnailsPage, and
  \ref ViewerPage

  The class \ref ViewerSizeConfig is a minor utility class used on the \ref
  ViewerPage.

  \ref CategoryItem represent the items on the \ref CategoryPage, plus some
  of the methods for adding and removing categories.

  <h2>The backend</h2>
  All settings on the configuration pages are stored in SettingsData, which
  is the interface the rest of the application uses.
**/
// vi:expandtab:tabstop=4 shiftwidth=4:
