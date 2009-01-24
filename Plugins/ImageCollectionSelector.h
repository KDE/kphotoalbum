#ifndef MYIMAGECOLLECTIONSELECTOR_H
#define MYIMAGECOLLECTIONSELECTOR_H

#include <config-kpa-kipi.h>
#include "Plugins/ImageCollection.h"
#include "Plugins/Interface.h"
#include <kdemacros.h>

namespace Plugins
{
/** This class should provide a widget for selecting one ore more image collection for plugins that want the user to
 *  select images.
 *
 * Since selecting images is all kphotoalbum is about ;-), this implementation just passes the images that are (or
 * would be) currently visible in thumbnail view - if some of them are selected, only selected ones, otherwise all.
 *
 * The widget shown is currently empty.
 *
 * Possible improvements:
 *  * show some description of the currently selected images instead of just nothing
 *  * give the user the possibility to group the selected images into image collections by some category: this would be
 *    useful as i.e. html export plugin uses the names of image collections as headlines and groups the images visually by
 *    image collection.
 */
class KDE_EXPORT ImageCollectionSelector :public KIPI::ImageCollectionSelector
{
public:
    ImageCollectionSelector(QWidget *parent, Interface *interface);
    virtual QList<KIPI::ImageCollection> selectedImageCollections() const;

protected:
    // just fake a selectionChanged event when first shown to make export plugin happy:
    virtual void showEvent(QShowEvent *event);

private:
    Interface *_interface;
    bool firstTimeVisible;
};

}

#endif /* MYIMAGECOLLECTIONSELECTOR_H */
