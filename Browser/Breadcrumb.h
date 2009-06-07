#ifndef BREADCRUMB_H
#define BREADCRUMB_H
#include <QList>
#include <QString>

namespace Browser
{

/**
 * \brief Information about a single breadcrumb
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This is basically a simple class to make the code for handling
 * breadcrumbs simpler. It encodes the following two informations about a
 * breadcrumb:
 * \li Is this a first breadcrumb (the result of going home e.g.)
 * \li which text should be shown for this breadcrumb.
 *
 */
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

