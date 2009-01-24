#include <config-kpa-kipi.h>
#ifdef HASKIPI
#include "Plugins/ImageCollectionSelector.h"

Plugins::ImageCollectionSelector::ImageCollectionSelector( QWidget *parent, Interface *interface )
    : KIPI::ImageCollectionSelector( parent )
{
    _interface = interface;
    firstTimeVisible = true;
}

QList<KIPI::ImageCollection> Plugins::ImageCollectionSelector::selectedImageCollections() const
{
    qDebug("selectedImageCollections");
    if ( _interface ) {
        KIPI::ImageCollection collection = _interface->currentSelection();
        if (!collection.isValid()) {
            collection = _interface->currentAlbum();
        }
        if (collection.isValid()) {
            QList<KIPI::ImageCollection> res;
            res.append(collection);
            return res;
        }
        // probably never happens:
        return _interface->allAlbums();
    }
    return QList<KIPI::ImageCollection>();
}

void Plugins::ImageCollectionSelector::showEvent(QShowEvent *event) {
    KIPI::ImageCollectionSelector::showEvent(event);
    if (firstTimeVisible) {
        // fake one selection change to make HTML Export Plugin believe there really is a selection:
        emit selectionChanged();
        firstTimeVisible = false;
    }
}

#endif // HASKIPI
