#include "listselect.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include "options.h"

class CompletableLineEdit :public QLineEdit {
public:
    CompletableLineEdit( QWidget* parent,  const char* name = 0 );
    void setListBox( QListBox* );
protected:
    virtual void keyPressEvent( QKeyEvent* ev );

private:
    QListBox* _listbox;
};

CompletableLineEdit::CompletableLineEdit( QWidget* parent, const char* name )
    :QLineEdit( parent, name )
{
}

void CompletableLineEdit::setListBox( QListBox* listbox )
{
    _listbox = listbox;
}

void CompletableLineEdit::keyPressEvent( QKeyEvent* ev )
{
    if ( !ev->text().isEmpty() && ev->text()[0].isPrint() )  {
        QLineEdit::keyPressEvent( ev );

        QString input = text();
        QListBoxItem* item = _listbox->findItem( input );
        if ( item )  {
            _listbox->setCurrentItem( item );
            _listbox->ensureCurrentVisible();
            setText( item->text() );
            setSelection( input.length(), item->text().length() - input.length() );
        }
    }
    else
        QLineEdit::keyPressEvent( ev );
}

ListSelect::ListSelect( QWidget* parent, const char* name )
    : QWidget( parent,  name )
{
    QVBoxLayout* layout = new QVBoxLayout( this,  6 );

    _label = new QLabel( this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );

    _lineEdit = new CompletableLineEdit( this );
    layout->addWidget( _lineEdit );

    _listBox = new QListBox( this );
    _listBox->setSelectionMode( QListBox::Multi );
    layout->addWidget( _listBox );

    _merge = new QCheckBox( "Merge",  this );
    layout->addWidget( _merge );

    _lineEdit->setListBox( _listBox );
    connect( _lineEdit, SIGNAL( returnPressed() ),  this,  SLOT( slotReturn() ) );
}

void ListSelect::setLabel( const QString& label )
{
    _textLabel = label;
    _label->setText( label );
}

void ListSelect::slotReturn()
{
    QString txt = _lineEdit->text();
    if ( txt == "" )
        return;

    QListBoxItem* item = _listBox->findItem( txt,  ExactMatch );
    if ( !item ) {
        item = new QListBoxText( _listBox, txt );
    }
    Options* options = Options::instance();
    options->addOption( _textLabel, txt);

    _listBox->setSelected( item,  true );
    _lineEdit->clear();
    _listBox->sort();
}

void ListSelect::insertStringList( const QStringList& list )
{
    _listBox->insertStringList( list );
}

QString ListSelect::label() const
{
    return _textLabel;
}

void ListSelect::setSelection( const QStringList& list )
{
    _listBox->clearSelection();

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QListBoxItem* item = _listBox->findItem( *it,  ExactMatch );
        if ( !item )  {
            _listBox->insertItem( *it );
            item = _listBox->findItem( *it,  ExactMatch );
        }
        _listBox->setSelected( item,  true );
    }
}

QStringList ListSelect::selection()
{
    QStringList list;
    for ( QListBoxItem* item = _listBox->firstItem(); item; item = item->next() )  {
        if ( item->isSelected() )
            list.append( item->text() );
    }
    return list;
}

void ListSelect::setShowMergeCheckbox( bool b )
{
    _merge->setEnabled( b );
}

bool ListSelect::merge() const
{
    return _merge->isChecked();
}
