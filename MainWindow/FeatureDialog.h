#ifndef FEATUREDIALOG_H
#define FEATUREDIALOG_H
#include <kdialogbase.h>
#include <qtextbrowser.h>

namespace MainWindow
{

class FeatureDialog : public KDialogBase {
    Q_OBJECT

public:
    FeatureDialog( QWidget* parent, const char* name = 0 );
};

class HelpBrowser :public QTextBrowser
{
public:
    HelpBrowser( QWidget* parent, const char* name = 0 );
    virtual void setSource( const QString& name );
};

}

#endif /* FEATUREDIALOG_H */

