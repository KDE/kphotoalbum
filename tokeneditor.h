#ifndef TOKENEDITOR_H
#define TOKENEDITOR_H

#include <kdialogbase.h>
#include <qvaluelist.h>
class QCheckBox;

class TokenEditor :public KDialogBase {
    Q_OBJECT

public:
    TokenEditor( QWidget* parent, const char* name = 0 );
    virtual void show();

protected slots:
    void selectAll();
    void selectNone();
    virtual void slotOk();

protected:
    QStringList tokensInUse();

private:
    QValueList<QCheckBox*> _cbs;
};


#endif /* TOKENEDITOR_H */

