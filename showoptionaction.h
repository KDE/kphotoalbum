#ifndef SHOWOPTIONACTION_H
#define SHOWOPTIONACTION_H
#include <qaction.h>

class ShowOptionAction :public QAction {
    Q_OBJECT

public:
    ShowOptionAction( const QString& optionGroup, QObject* parent, const char* name = 0 );

protected slots:
    void slotToggled( bool b );

signals:
    void toggled( const QString& optionGroup, bool b );

private:
    QString _optionGroup;
};


#endif /* SHOWOPTIONACTION_H */

