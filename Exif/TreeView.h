#ifndef EXIFTREEVIEW_H
#define EXIFTREEVIEW_H

#include <qlistview.h>
#include "Utilities/Set.h"

namespace Exif{

class TreeView : public QListView {
    Q_OBJECT

public:
    TreeView( const QString& title, QWidget* parent, const char* name = 0 );
    Set<QString> selected();
    void setSelected( const Set<QString>& selected );
    void reload();

protected slots:
    void toggleChildren( QListViewItem* );
};

}



#endif /* EXIFTREEVIEW_H */

