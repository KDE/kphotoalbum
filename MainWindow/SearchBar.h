#ifndef SEARCHBAR_H
#define SEARCHBAR_H
#include <ktoolbar.h>
class KLineEdit;
class KMainWindow;

namespace MainWindow
{

class SearchBar :public KToolBar {
    Q_OBJECT

public:
    SearchBar( KMainWindow* parent, const char* name = 0 );

protected slots:
    void reset();

protected:
    virtual bool eventFilter( QObject* watched, QEvent* e );

signals:
    void textChanged( const QString& );
    void returnPressed();
    void scrollLine( int direction );
    void scrollPage( int direction );

private:
    KLineEdit* _edit;
    QWidget* _browser;
};

}


#endif /* SEARCHBAR_H */

