#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <qwidget.h>
#include <qstringlist.h>
class QListBox;
class QLabel;
class QCheckBox;
class CompletableLineEdit;
class ImageInfo;
class QListBoxItem;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( QWidget* parent,  const char* name = 0 );
    void setLabel( const QString& label );
    QString label() const;
    QString text() const;
    void setText( const QString& );
    void insertStringList( const QStringList& list );
    void setSelection( const QStringList& list );
    QStringList selection();
    void setShowMergeCheckbox( bool b );
    bool merge() const;

    enum Mode {INPUT, SEARCH};
    void setMode( Mode );
    bool matches( ImageInfo* info );

    QWidget* firstTabWidget() const;
    QWidget* lastTabWidget() const;

public slots:
    void slotReturn();

signals:
    void deleteOption( const QString& optionGroup, const QString& which);
    void renameOption( const QString& optionGroup, const QString& oldValue, const QString& newValue );

protected slots:
    void itemSelected( QListBoxItem* );
    void showContextMenu( QListBoxItem*, const QPoint& );

private:
    QLabel* _label;
    QString _textLabel;
    CompletableLineEdit* _lineEdit;
    QListBox* _listBox;
    QCheckBox* _merge;
    Mode _mode;
};

#endif /* LISTSELECT_H */

