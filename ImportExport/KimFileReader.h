#ifndef KIMFILEREADER_H
#define KIMFILEREADER_H
#include <QPixmap>
#include <QString>
class KArchiveDirectory;
class KZip;

namespace ImportExport {

class KimFileReader
{
public:
    KimFileReader();
    ~KimFileReader();
    bool open(const QString& fileName);
    QByteArray indexXML();
    QPixmap loadThumbnail( QString fileName );
    QByteArray loadImage( const QString& fileName );


private:
    QString m_fileName;
    KZip* _zip;
    const KArchiveDirectory* _dir;
};

}

#endif /* KIMFILEREADER_H */

