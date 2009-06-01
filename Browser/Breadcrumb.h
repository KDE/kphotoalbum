#ifndef BREADCRUMB_H
#define BREADCRUMB_H
#include <QList>
#include <QString>

namespace Browser
{

class Breadcrumb
{
public:
    static Breadcrumb empty();
    static Breadcrumb home();
    Breadcrumb( const QString& text, bool isBeginning = false );
    QString text() const;
    bool isBeginning() const;
    bool operator==( const Breadcrumb& other ) const;
    bool operator!=( const Breadcrumb& other ) const;

private:
    int _index;
    bool _isBeginning;
    QString _text;
    static int _count;
};

}

#endif /* BREADCRUMB_H */

