#ifndef SEARCHBAR_H
#define SEARCHBAR_H
#include <ktoolbar.h>
class KLineEdit;
class KMainWindow;

class SearchBar :public KToolBar {
    Q_OBJECT

public:
    SearchBar( KMainWindow* parent, const char* name = 0 );

protected slots:
    void reset();

signals:
    void textChanged( const QString& );

private:
    KLineEdit* _edit;
};


#endif /* SEARCHBAR_H */

