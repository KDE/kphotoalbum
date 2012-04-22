/**
 * \namespace Viewer
 * \brief Viewer used for displaying images and videos
 *
 * This class implements the viewer used to display images and videos.
 *
 * The class consists of these components:
 * <ul>
 * <li>\ref ViewerWidget - This is the topmost widget used as the viewer.
 * <li>\ref AbstractDisplay, \ref ImageDisplay, \ref VideoDisplay and \ref TextDisplay - Widgets hierarchy which takes care of the actual displaying of content.
 * <li> \ref ViewHandler - Handler which interprets mouse gestures.
 * <li> \ref InfoBox - Widget implementing the informatiom box
 * <li> \ref SpeedDisplay - Widget implementing the toplevel display used when adjusting slideshow speed.
 * <li> \ref VideoShooter - Utility class helping with taking a screenshot of a video frame
 * </ul>
 */
