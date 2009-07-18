/**
  \namespace ThumbnailView
  \brief  The classes in this namespace makes up the thumbnail window

  <h2>Design Decisions</h2>
  \li \ref model-view-in-thumbnail

  <h2>Base structure and API</h2>
  The only class that should be seen from the outside is the class \ref
  ThumbnailFacade. This class has delegates for the methods on the various
  object that the outside need to know. Please be careful not to expose
  anything, except though this class.

  The thumbnail viewer was likely one of the very first pieces of code that
  was written in KPhotoAlbum, and consequently also one of the pieces of
  code that just grew larger and larger organically. During a large
  refactoring session in July 2009, this was cleaned up, but a number of
  objects are still very tightly connected, namely the widget itself (\ref
  ThumbnailWidget), its painting code (\ref ThumbnailPainter), its data
  model (\ref ThumbnailModel), and its event handlers.

  To avoid that all these classes needed to be set up with pointers to each
  other, a factory (\ref ThumbnailFactory) was created, from which any of
  these object could get to the other. To avoid outside
  objects to misuse this factory to get to the objects, they were explicit
  constructed with the factory by the object who owned the pointers to all of
  the participants, namely \ref ThumbnailFacade, and as the \ref
  ThumbnailFactory only is an interface, it would not help the outside in.

  To ease the jump between the involved objects, and to make it more
  explicit which objects participated, and what they accessed from each
  other, they all inherited from \ref ThumbnailComponent which offers
  methods like \ref ThumbnailComponent::model(), \ref
  ThumbnailComponent::widget() etc.

  The core part of this module consist of these classes:
  \li \ref ThumbnailFacade - The API for the other modules in KPhotoAlbum.
  \li \ref ThumbnailWidget - This is the widget that are put into the main
  windows gui.
  \li \ref ThumbnailModel - This is the underlying model of the
  widget. Here you will find methods to access and alter the selection, the
  stack, the list of images etc.
  \li \ref ThumbnailPainter - Here are all the code for painting the
  widget.
  \li \ref CellGeometry - This is where you will find the logic for
  calculating the sizes of cells, the text height etc.
  \li \ref Cell - This is a simple class with a (row,column) content.

  <h2>Thumbnail Handling</h2>
  \li \ref ThumbnailRequest - When the browser recognizes that it needs a
  thumbnail, it will ask the \ref ImageManager to create one. This could of
  course result in a huge amount of requests that are pending but never
  needed anyway, in the case where the user presses page down to scroll by
  lots of pages. To solve that problem, the \ref ImageManager can ask the
  request if it is still needed, at the time when it is about to load the
  thumbnail in question. To implement this, we need to subclass
  \ref ImageManager::ImageRequest - that's exactly what this class is about.

  \li \ref ThumbnailCache - To make the scrolling as smooth as possible, this
  class will try to preheat a cache of thumbnails that we expect would be
  needed at the next scroll action.

  \li \ref ThumbnailBuilder - It is possible from the menus to ask the
  application to rebuild all its thumnails on disk. This class will do that
  for us.

  <h2>Mouse Interaction</h2>
  \li \ref MouseInteraction The logic behind mouse gestures are slightly
  complicated, and has been the root of a lot of trouble in the past of
  KPhotoAlbum. Therefore the mouse handling has been split into a number of
  classes on their own. Mouse events are delegated from the \ref
  ThumbnailWidget to these classes. The super class for these sub classes
  are \ref MouseInteraction.

  \li \ref MouseTrackingInteraction - This is by far the simplest of all of the
  interaction classes, it is in action when no mouse button are down, and
  takes care of updating the status bar with information about which
  thumbnail the cursor is over.

  \li \ref SelectionInteraction - This interaction takes care of selecting items
  in the thumbnail view, it is active when the left mouse button has been
  pressed down, and is dragged over items.

  \li \ref GridResizeInteraction - This interaction is active when the user is
  resizing the grid. It is active when the middle mouse button has been
  pressed down.

  \li \ref ThumbnailDND - Reordering images is done using the drag and drop
  mechanism of Qt. The implementation of these event handlers are done in
  this class.

  <h2>Keyboard Interaction</h2>
  \li Keyboard event handlers are implemented in \ref KeyboardEventHandler.

  <h2>Tooltips</h2>
  \li \ref ThumbnailToolTip - In the thumbanil viewer, it is possible to show
  some tooltips which shows details about the thumbnail the cursor is on
  top of. This is implemented using this class.

**/

/**
   \page model-view-in-thumbnail Why are we not using Model/View to implement the Thumbnail Viewer?

   The Model/View framework seems like an obvious choice for implementing
   the thumbnail viewer, still we don't use it, why?

   Well, first of all, when the thumbnail viewer was initially implemented,
   no model/view framework existed - this was first introduced in Qt4.

   Still, I did give it a worthy shot at rewriting the thumbnail viewer to
   the model/view framework, and I was all excited about the thought of
   getting rid of especially the \ref ThumbnailView::SelectionInteraction
   class. Unfortunately I hit an problem I could not find a way to resolve,
   even after having studied the code of QListView and QAbstractItemView in
   details. The obstacle was selection. KPhotoAlbums concept of row
   selection was simply not possible with QListView, and the only way I
   could find forward with that was to reimplement mouse handling for the
   model/view framework, which seemed to me like a dead end.

   Jesper, July 2009.
*/
