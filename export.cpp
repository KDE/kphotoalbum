#include "export.h"
#include <kfiledialog.h>
#include <kzip.h>
#include <qfileinfo.h>
#include <time.h>
#include "util.h"

void Export::imageExport( const ImageInfoList& list )
{
    QString zipFile = KFileDialog::getSaveFileName( QString::null, QString::fromLatin1( "*.kim|KimDaBa export files" ), 0 );
    if ( zipFile.isNull() )
        return;

    KZip zip( zipFile );
    zip.open( IO_WriteOnly );

    QMap<QString,QString> map;
    int index = 0;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file =  (*it)->fileName();
        QString zippedName = QString::fromLatin1( "image%1.%2" ).arg( Util::pad(6,++index) ).arg( QFileInfo( file).extension() );
        map.insert( file, zippedName );

        if ( QFileInfo( file ).isSymLink() )
            file = QFileInfo(file).readLink();

        zip.addLocalFile( file, zippedName );
    }

    QCString indexml = createIndexXML( list, map );
    time_t t;
    time(&t);
    zip.writeFile( QString::fromLatin1( "index.xml" ), QString::null, QString::null, indexml.size()-1, 0444, t, t, t, indexml.data() );
    zip.close();
}

QCString Export::createIndexXML( const ImageInfoList& list, const QMap<QString, QString>& map)
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );

    QDomElement top = doc.createElement( QString::fromLatin1( "KimDaBa-export" ) );
    doc.appendChild( top );


    for( ImageInfoListIterator it( list ); *it; ++it ) {
        QString file = (*it)->fileName();
        QString mappedFile = map[file];
        QDomElement elm = (*it)->save( doc );
        elm.setAttribute( QString::fromLatin1( "file" ), mappedFile );
        top.appendChild( elm );
    }
    return doc.toCString();
}
