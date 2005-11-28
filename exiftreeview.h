#ifndef EXIFTREEVIEW_H
#define EXIFTREEVIEW_H

#include <qlistview.h>
#include "set.h"

class ExifTreeView : public QListView {
    Q_OBJECT

public:
    ExifTreeView( const QString& title, QWidget* parent, const char* name = 0 );
    Set<QString> selected();
    void setSelected( const Set<QString>& selected );

protected slots:
    void toggleChildren( QListViewItem* );
};


#endif /* EXIFTREEVIEW_H */

