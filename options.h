#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>

class Options {
public:
    static Options* instance();
    static bool configFileExists();
    static void setConfFile( const QString& file );

    void setUse4To3Ratio( bool );
    bool use4To3Ratio() const;

    void setThumbSize( int );
    int thumbSize() const;

    void setCacheThumbNails( bool );
    bool cacheThumbNails() const;

    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QString& value );
    QStringList optionValue( const QString& key ) const;

    void setTrustTimeStamps( bool );
    bool trustTimeStamps() const;

    QStringList dataDirs() const;
    void save();

    QString imageDirectory() const;
    void setImageDirecotry( const QString& directory );

    // Options for the Viewer
    enum Position { Bottom, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    bool showInfoBox() const;
    bool showDescription() const;
    bool showDate() const;
    bool showLocation() const;
    bool showNames() const;

    void setShowInfoBox(bool b);
    void setShowDescription(bool b);
    void setShowDate(bool b);
    void setShowLocation(bool b);
    void setShowNames(bool b);

    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );


private:
    Options();
    ~Options() {};
    static Options* _instance;
    static QString _confFile;

    int _thumbSize,  _imageCacheSize;
    bool _cacheThumbNails,  _use4To3Ratio, _trustTimeStamps;
    QMap<QString, QStringList> _options;
    QString _imageDirectory;

    Position _infoBoxPosition;
    bool _showInfoBox, _showDescription, _showDate, _showNames, _showLocation;

};

#endif /* OPTIONS_H */

