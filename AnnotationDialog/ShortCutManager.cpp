#include "ShortCutManager.h"
#include "ListSelect.h"
#include <QShortcut>
#include <QDockWidget>

/**
 * Register the dock widget for getting a shortcut. its buddy will get the
 * actual focus when the shortcut is execute.
 */
void AnnotationDialog::ShortCutManager::addDock( QDockWidget* dock, QWidget* buddy )
{
    m_docks.append( qMakePair(dock,buddy) );
}

void AnnotationDialog::ShortCutManager::addLabel( QLabel* label )
{
    m_labelWidgets.append(label);
}

void AnnotationDialog::ShortCutManager::setupShortCuts()
{
    Q_FOREACH( const DockPair& pair, m_docks ) {
        QDockWidget* dock = pair.first;
        QWidget* widget = pair.second;
        QString title = dock->windowTitle();
        for ( int index = 0; index < title.length(); ++index ) {
            const QChar ch = title[index].toLower();
            if ( !m_taken.contains(ch) ) {
                m_taken.insert( ch );
                dock->setWindowTitle( title.left(index) + QString::fromLatin1("&") + title.mid(index) );
                new QShortcut( QString::fromLatin1( "Alt+")+ch , widget, SLOT( setFocus() ) );
                break;
            }
        }
    }

    Q_FOREACH( QLabel* label, m_labelWidgets ) {
        const QString title = label->text();
        for ( int index = 0; index < title.length(); ++index ) {
            const QChar ch = title[index].toLower();
            if ( !m_taken.contains(ch) ) {
                m_taken.insert( ch );
                label->setText( title.left(index) + QString::fromLatin1("&") + title.mid(index) );
                break;
            }
        }
    }

}


/**
 * Search for & in the text, and if found register the character after the ampersand as a shortcut
 * This is needed as the OK and Cancel button will get a shortcut by KDE,
 * despite an attempt at telling it not too.
 */
void AnnotationDialog::ShortCutManager::addTaken( const QString& text )
{
    const int index = text.indexOf(QChar::fromLatin1('&'));
    if ( index != -1 )
        m_taken.insert( text[index+1].toLower() );
}
