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
    static bool hasVideoSupport( const QString& mimeType );

protected:
    static bool hasKIPISupport();
    static bool hasSQLDBSupport();
    static bool hasEXIV2Support();
    static bool hasEXIV2DBSupport();
};

class HelpBrowser :public QTextBrowser
{
public:
    HelpBrowser( QWidget* parent, const char* name = 0 );
    virtual void setSource( const QString& name );
};

}

#endif /* FEATUREDIALOG_H */

