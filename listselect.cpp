#include "listselect.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include "options.h"
#include "imageinfo.h"

class CompletableLineEdit :public QLineEdit {
public:
    CompletableLineEdit( QWidget* parent,  const char* name = 0 );
    void setListBox( QListBox* );
    void setMode( ListSelect::Mode mode );
protected:
    virtual void keyPressEvent( QKeyEvent* ev );

private:
    QListBox* _listbox;
    ListSelect::Mode _mode;
};


CompletableLineEdit::CompletableLineEdit( QWidget* parent, const char* name )
    :QLineEdit( parent, name )
{
}

void CompletableLineEdit::setListBox( QListBox* listbox )
{
    _listbox = listbox;
}

void CompletableLineEdit::setMode( ListSelect::Mode mode ) {
    _mode = mode;
}

// Better hoope this monster works....
void CompletableLineEdit::keyPressEvent( QKeyEvent* ev )
{
    if ( ev->key() == Key_Return )  {
        QLineEdit::keyPressEvent( ev );
        return;
    }
    qDebug("%x", ev->key());


    if ( !ev->text().isEmpty() && ev->text()[0].isPrint() )  {
        bool special = ( ev->text() == "&" || ev->text() == "|" || ev->text() == "!" /* || ev->text() == "(" */ );
        if ( _mode == ListSelect::INPUT && special )  {
            // Don't insert the special character.
            return;
        }

        QString content = text();
        int cursorPos = cursorPosition();

        // Space, &,|, or ! should result in the current item being inserted
        if ( _mode == ListSelect::SEARCH && (special || ev->text() == " " ) )  {
            QString txt = text().left(cursorPos) + ev->text() + text().mid( cursorPos );
            setText( txt );
            setCursorPosition( cursorPos + ev->text().length() );
            deselect();

            // Select the item in the listbox - not perfect but acceptable for now.
            int start = txt.findRev( QRegExp("[!&|]"), cursorPosition() -2 ) +1;
            QString input = txt.mid( start, cursorPosition()-start-1 );
            input = input.stripWhiteSpace();

            QListBoxItem* item = _listbox->findItem( input );
            if ( item )
                _listbox->setSelected( item, true );

            return;
        }


        QLineEdit::keyPressEvent( ev );


        // Find the text of the current item
        int start = 0;
        QString input = text();
        if ( _mode == ListSelect::SEARCH )  {
            input = input.left( cursorPosition() );
            start = input.findRev( QRegExp("[!&| ]") ) +1;
            input = input.mid( start );
        }

        // Find the text in the listbox
        QListBoxItem* item = _listbox->findItem( input );
        if ( !item && _mode == ListSelect::SEARCH )  {
            // revert
            setText( content );
            setCursorPosition( cursorPos );
            item = _listbox->findItem( input );
        }

        if ( item )  {
            _listbox->setCurrentItem( item );
            _listbox->ensureCurrentVisible();

            QString txt = text().left(start) + item->text() + text().mid( cursorPosition() );
            setText( txt );
            setSelection( start + input.length(), item->text().length() - input.length() );
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
    connect( _listBox, SIGNAL( clicked( QListBoxItem*  ) ),  this,  SLOT( itemSelected( QListBoxItem* ) ) );
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
    if ( _mode == INPUT )  {
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
    _lineEdit->clear();
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

void ListSelect::setMode( Mode mode)
{
    _mode = mode;
    _lineEdit->setMode( mode );
}

bool ListSelect::matches( ImageInfo* info )
{
    // PENDING(blackie) to simple algorithm for matching, could be improved with parentheses.
    QString matchText = _lineEdit->text();
    if ( matchText.isEmpty() )
        return true;

    QStringList orParts = QStringList::split( "|", matchText );
    bool orTrue = false;
    for( QStringList::Iterator itOr = orParts.begin(); itOr != orParts.end(); ++itOr ) {
        QStringList andParts = QStringList::split( "&", *itOr );
        bool andTrue = true;
        for( QStringList::Iterator itAnd = andParts.begin(); itAnd != andParts.end(); ++itAnd ) {
            QString str = *itAnd;
            bool negate = false;
            QRegExp regexp( "^\\s*!\\s*(.*)$" );
            if ( regexp.exactMatch( str ) )  {
                negate = true;
                str = regexp.cap(1);
            }
            str = str.stripWhiteSpace();
            bool found = info->hasOption( _textLabel,  str );
            andTrue &= ( negate ? !found : found );
        }
        orTrue |= andTrue;
    }

    return orTrue;
}

QWidget* ListSelect::firstTabWidget() const
{
    return _lineEdit;
}

QWidget* ListSelect::lastTabWidget() const
{
    return _lineEdit;
}

QString ListSelect::text() const
{
    return _lineEdit->text();
}

void ListSelect::setText( const QString& text )
{
    _lineEdit->setText( text );
    _listBox->clearSelection();
}

void ListSelect::itemSelected( QListBoxItem* item )
{
    if ( _mode == SEARCH )  {
        QString txt = item->text();
        QString res;
        QRegExp regEnd( "\\s*[&|!]\\s*$" );
        QRegExp regStart( "^\\s*[&|!]\\s*" );
        if ( item->isSelected() )  {
            int index = _lineEdit->cursorPosition();
            QString start = _lineEdit->text().left(index);
            QString end =  _lineEdit->text().mid(index);

            res = start;
            if ( !start.isEmpty() && !start.contains( regEnd ) )
                 res += " & ";
            res += txt;
            if ( !end.isEmpty() && !end.contains( regStart ) )
                res += " & ";
            res += end;
        }
        else {
            int index = _lineEdit->text().find( txt );
            if ( index == -1 )
                return;

            QString start = _lineEdit->text().left(index);
            QString end =  _lineEdit->text().mid(index + txt.length() );
            if ( start.contains( regEnd ) )
                start.replace( regEnd, "" );
            else
                end.replace( regStart,  "" );

            res = start + end;
        }
        _lineEdit->setText( res );
    }
}


