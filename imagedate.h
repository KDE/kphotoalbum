#ifndef IMAGEDATE_H
#define IMAGEDATE_H
#include <qstring.h>

class ImageDate {
public:
    ImageDate();
    ImageDate( int day, int month, int year );
    int year() const;
    int month() const;
    int day() const;
    void setYear( int );
    void setMonth( int );
    void setDay( int );
    bool operator<=( ImageDate& other );
    bool isNull() const;
    QString toString();
    operator QString() { return toString(); }

private:
    int _year, _month, _day;
};

#endif /* IMAGEDATE_H */

