#ifndef BROWSER_H
#define BROWSER_H
#include <qiconview.h>
class FolderAction;
class ImageSearchInfo;

class Browser :public QIconView {
    Q_OBJECT
    friend class ImageFolderAction;

public:
    Browser( QWidget* parent, const char* name = 0 );
    void addSearch( ImageSearchInfo& info );
    static Browser* theBrowser();
    void load( const QString& optionGroup, const QString& value );

public slots:
    void back();
    void forward();
    void go();
    void home();

signals:
    void canGoBack( bool );
    void canGoForward( bool );
    void showingOverview();


protected slots:
    void init();
    void select( QIconViewItem* item );
    void reload();

protected:
    void addItem( FolderAction* );
    void emitSignals();

private:
    static Browser* _instance;
    QValueList<FolderAction*> _list;
    int _current;
};


#endif /* BROWSER_H */

