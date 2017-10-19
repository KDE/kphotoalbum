
#ifndef EXIFTREEVIEW_H
#define EXIFTREEVIEW_H

#include "Utilities/StringSet.h"
#include <QTreeWidget>

namespace Exif {
using Utilities::StringSet;

class TreeView : public QTreeWidget {
    Q_OBJECT

public:
    TreeView( const QString& title, QWidget* parent );
    StringSet selected();
    void setSelectedExif( const StringSet& selected );
    void reload();

protected slots:
    void toggleChildren( QTreeWidgetItem* );
};

}



#endif /* EXIFTREEVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
