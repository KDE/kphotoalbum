#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>

class Options {
public:
    static Options* instance();

    void setUse4To3Ratio( bool );
    bool use4To3Ratio() const;

    void setThumbWidth( int );
    int thumbWidth() const;

    void setThumbHeight( int );
    int thumbHeight() const;

    void setNumThreads( int );
    int numThreads() const;

    void setCacheThumbNails( bool );
    bool cacheThumbNails() const;

    void setOption( const QString& key,  const QStringList& value );
    QStringList optionValue( const QString& key ) const;

    void setTrustTimeStamps( bool );
    bool trustTimeStamps() const;

    QStringList dataDirs() const;
    void save();

private:
    Options();
    ~Options() {};
    static Options* _instance;

    int _thumbWidth, _thumbHeight,  _imageCacheSize,  _numThreads;
    bool _cacheThumbNails,  _use4To3Ratio, _trustTimeStamps;
    QMap<QString, QStringList> _options;
};

#endif /* OPTIONS_H */

