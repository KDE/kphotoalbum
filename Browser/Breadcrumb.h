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

private:
    bool _isBeginning;
    QString _text;
};

}

#endif /* BREADCRUMB_H */

