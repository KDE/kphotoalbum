#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <qwidget.h>
class QLineEdit;
class QListBox;
class QLabel;
class QCheckBox;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( QWidget* parent,  const char* name = 0 );
    void setLabel( const QString& label );
    QString label() const;
    void insertStringList( const QStringList& list );
    void setSelection( const QStringList& list );
    QStringList selection();
    void setShowMergeCheckbox( bool b );
    bool merge() const;

protected slots:
    void slotReturn();
    void completeLineEdit( const QString& );

private:
    QLabel* _label;
    QString _textLabel;
    QLineEdit* _lineEdit;
    QListBox* _listBox;
    QCheckBox* _merge;
};

#endif /* LISTSELECT_H */

