#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <qwidget.h>
class QLineEdit;
class QListBox;
class QLabel;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( QWidget* parent,  const char* name = 0 );
    void setLabel( const QString& label );
    QString label() const;
    void insertStringList( const QStringList& list );
    void setSelection( const QStringList& list );
    QStringList selection();

protected slots:
    void slotReturn();

private:
    QLabel* _label;
    QString _textLabel;
    QLineEdit* _lineEdit;
    QListBox* _listBox;
};

#endif /* LISTSELECT_H */

