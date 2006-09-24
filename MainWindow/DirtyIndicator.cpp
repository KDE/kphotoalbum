#include "DirtyIndicator.h"
#include <kiconloader.h>

static MainWindow::DirtyIndicator* _instance = 0;
bool MainWindow::DirtyIndicator::_autoSaveDirty = false;
bool MainWindow::DirtyIndicator::_saveDirty = false;

MainWindow::DirtyIndicator::DirtyIndicator( QWidget* parent )
    :QLabel( parent )
{
    _dirtyPix = QPixmap( SmallIcon( QString::fromLatin1( "3floppy_unmount" ) ) );
    setFixedWidth( _dirtyPix.width() );
    _instance = this;

    // Might have been marked dirty even before the indicator had been created, by the database searching during loading.
    if ( _saveDirty )
        markDirty();
}

void MainWindow::DirtyIndicator::markDirty()
{
    _saveDirty = true;
    _autoSaveDirty = true;
    if ( _instance ) {
        _instance->setPixmap( _instance->_dirtyPix );
        emit _instance->dirty();
    }
}

void MainWindow::DirtyIndicator::autoSaved()
{
    _autoSaveDirty= false;
}

void MainWindow::DirtyIndicator::saved()
{
    _autoSaveDirty = false;
    _saveDirty = false;
    setPixmap( QPixmap() );
}

bool MainWindow::DirtyIndicator::isSaveDirty() const
{
    return _saveDirty;
}

bool MainWindow::DirtyIndicator::isAutoSaveDirty() const
{
    return _autoSaveDirty;
}

