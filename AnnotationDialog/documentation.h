// SPDX-FileCopyrightText: 2009 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

// krazy:skip
/**
  \namespace AnnotationDialog
  \brief The dialog used for tagging images

  <h2>The Annotation %Dialog</h2>

  The word annotation and tagging is used for the same thing in KPhotoAlbum.

  The dialog itself is implemented in the class \ref Dialog. The
  responsibility for the Dialog class is among other things:
  \li Setting up the GUI for the annotation dialog.
  \li Handling the windows that can be moved around, torn off, hidden etc.
  \li Loading/saving the state of the windows
  \li Being able to undo changes

  Please notice the dialog is used both for tagging and for searching.

  <h2>Category Tagging</h2>
  The category tagging is visually the line edit with a listbox under. It
  is handled with the classes \ref ListSelect (which is a widget with both
  the line edit and the listbox) and \ref CompletableLineEdit (which is the
  class for the line edit)


  The content of the list boxes are shown and hidden in two different
  ways:
  \li When typing text in the line edit (managed by \ref ListViewTextMatchHider)
  \li When asking to see only item checked (managed by \ref ListViewCheckedHider)

  Both of the above classes are subclass of \ref ListViewItemHider.

  Each category has an action set up for toggling the visibility of non-checked
  items, but that should of course be the same for all
  categories. To control this a singleton exists namely \ref ShowSelectionOnlyManager.

  <h2>Other Component of the dialog</h2>
  The image preview is an instance of the class \ref ImagePreview, and the date
  editor is an instance of the class \ref DateEdit. The later is copied
  from KDE and (IIRC) adapted to handle KPhotoAlbum's fuzzy dates (this
  image is from 2000). Actually I think this is handled differently in
  KPhotoAlbum now, so that copy could likely go away.

  <h2>Shortcuts</h2>
  KDE will by default setup shortcuts for application, but they are not
  that useful for this particular dialog, as we want the \ref ListSelect's
  to have the first pick on identifiers. We therefore have the class \ref
  ShortCutManager, which takes care of this.

**/
// vi:expandtab:tabstop=4 shiftwidth=4:
