#ifndef DELETEFILELISTDIALOG_H
#define DELETEFILELISTDIALOG_H

#include <kdialogbase.h>
#include <qstringlist.h>

class DeleteThumbnailsDialog :public KDialogBase {
    Q_OBJECT

public:
    DeleteThumbnailsDialog( QWidget* parent, const char* name = 0 );

protected slots:
    void slotDeleteFiles();
    void findThumbnails( const QString& directory );

private:
    QStringList _files;
};


#endif /* DELETEFILELISTDIALOG_H */

