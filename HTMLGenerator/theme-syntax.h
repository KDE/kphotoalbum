// krazy:skip
//  SPDX-FileCopyrightText: 2009 Jesper K. Pedersen <blackie@kde.org>
//
//  SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL OR CC-BY-SA-4.0
/**
   \page html-themes HTML themes

   Static web pages can be generated with selectable sized images.
   HTMLGenerator allows using of themes to produce differently styled web
   pages.

   Themes use template files that are used as basis for static HTML files.
   There are currently two template files used:
   \li mainpage.html
   \li imagepage.html

   Maingpage is used to generate index.html and image size specific index
   pages ( index-800x600.html ). imagepage.html is used to create one HTML
   page for each image and for each requested image size.

   Any other files within a theme directory are copied as is to the target
   web page. This way javascript, style files and theme specific images can
   easily be used.

   One more file included in a theme is file called kphotoalbum.theme. This
   file contains information about the theme author and theme name.

   <h2>Mainpage</h2>
   Template file mainpage.html can use the following tags that are then
   repplaced by KPA   with export specific data:
   \li **DESCRIPTION**

   Description given in configuration dialog

   \li **TITLE**

   Title given in configuration dialog

   \li **COPYRIGHT**

   Copyright owner / author given in configuration dialog

   \li **KIMFILE**

   Link to Kim file that can be generated for other people to import KPA
   keywords and other data associated with each image.

   \li **THUMBNAIL-TABLE**

   HTML formatted table containing all thumbnails to be exported to HTML
   page.

   \li **JSIMAGES**

   JavaScript formatted array initialization containing all images to be
   exported to HTML page. JS variables size and tsize are initialized to
   reflect sizes of images and thumbnails within the selected index page.
   Each array element consists of full sized image name (of currently
   selected image size), thumbnail image name, link to image specific HTML
   page and description field (containing information selected in HTML
   configuration dialog).

   \li **FIRST**

   Page name for first image in album.

   \li **LAST**

   Page name for last image in album.

   \li **RESOLUTIONS**

   Selected resolutions within the page with links to respective index
   pages.


   <h2>Imagepage</h2>
   Template file imagepage.html can use the following tags that are then
   replaced by KPA with export specific data:
   \li **TITLE**

   Image file name.

   \li **IMAGE_OR_VIDEO**

   Image or video with a link to next page.

   \li **PREV**

   Link to previous page.

   \li **PREVFILE**

   Name of previous file.

   \li **NEXT**

   Link to next page.

   \li **NEXTFILE**

   Name of next file.

   \li **INDEX**

   Link to index file.

   \li **INDEXFILE**

   Name of index file.

   \li **NEXTPAGE**

   Next page or index page if current page is last.

   \li **RESOLUTIONS**

   Selected resolutions within the page with links to respective index
   pages.

   \li **COPYRIGHT**

   Copyright owner / author given in configuration dialog

   \li **DESCRIPTION**
   Description contains what is selected to be included in
   HTMLGenerator dialog. E.g. keywords and image description.

**/
// vi:expandtab:tabstop=4 shiftwidth=4:
