#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>

class Options {
public:
    static Options* instance();

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

private:
    Options();
    ~Options() {};
    static Options* _instance;

    int _thumbSize,  _imageCacheSize;
    bool _cacheThumbNails,  _use4To3Ratio, _trustTimeStamps;
    QMap<QString, QStringList> _options;
};

#endif /* OPTIONS_H */

