#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>

class Options {
public:
    static Options* instance();
    static bool configFileExists();
    static void setConfFile( const QString& file );

    void setThumbSize( int );
    int thumbSize() const;

    void setCacheThumbNails( bool );
    bool cacheThumbNails() const;

    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QString& value );
    void removeOption( const QString& key, const QString& value );
    QStringList optionValue( const QString& key ) const;

    void setTrustTimeStamps( bool );
    bool trustTimeStamps() const;

    QStringList dataDirs() const;
    void save();

    QString imageDirectory() const;
    void setImageDirecotry( const QString& directory );

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    // Options for the Viewer
    enum Position { Bottom = 0, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    bool showInfoBox() const;
    bool showDrawings() const;
    bool showDescription() const;
    bool showDate() const;
    bool showLocation() const;
    bool showNames() const;
    bool showKeyWords() const;

    void setShowInfoBox(bool b);
    void setShowDrawings(bool b);
    void setShowDescription(bool b);
    void setShowDate(bool b);
    void setShowLocation(bool b);
    void setShowNames(bool b);
    void setShowKeyWords( bool b );

    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );


private:
    Options();
    ~Options() {};
    static Options* _instance;
    static QString _confFile;

    int _thumbSize,  _imageCacheSize;
    bool _cacheThumbNails, _trustTimeStamps, _markNew;
    QMap<QString, QStringList> _options;
    QString _imageDirectory, _htmlBaseDir, _htmlBaseURL;

    Position _infoBoxPosition;
    bool _showInfoBox, _showDrawings, _showDescription, _showDate, _showNames, _showLocation, _showKeyWords;

};

#endif /* OPTIONS_H */

