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
    static bool hasAllFeaturesAvailable();
    static QString featureString();

protected:
    static bool hasKIPISupport();
    static bool hasSQLDBSupport();
    static bool hasEXIV2Support();
    static bool hasEXIV2DBSupport();
    static bool hasVideoSupport( const QString& mimeType );
};

class HelpBrowser :public QTextBrowser
{
public:
    HelpBrowser( QWidget* parent, const char* name = 0 );
    virtual void setSource( const QString& name );
};

}

#endif /* FEATUREDIALOG_H */

