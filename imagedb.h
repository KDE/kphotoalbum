#ifndef IMAGEDB_H
#define IMAGEDB_H
#include "imageinfo.h"
#include "imagesearchinfo.h"
#include <qdict.h>

class ImageDB :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    int totalCount() const;
    void search( const ImageSearchInfo& info, int from = -1, int to = -1 );
    int count( const ImageSearchInfo& info );
    int countItemsOfOptionGroup( const QString& group );
    void renameOptionGroup( const QString& oldName, const QString newName );

    QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group );
    ImageInfoList& images() { return _images; }
    ImageInfoList& clipboard() { return _clipboard; }
    bool isClipboardEmpty();

public slots:
    void load();
    void save( const QString& fileName );

signals:
    void matchCountChange( int, int, int );
    void searchCompleted();

protected:
    void loadExtraFiles( const QDict<void>& loadedFiles, QString directory );
    void load( const QString& filename, QDomElement elm );
    void checkForBackupFile();
    int count( const ImageSearchInfo& info, bool makeVisible, int from, int to );

private:
    ImageDB();
    static ImageDB* _instance;

    ImageInfoList _images;
    ImageInfoList _clipboard;
};


#endif /* IMAGEDB_H */

