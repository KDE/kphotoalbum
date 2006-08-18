#include "ShowSelectionOnlyManager.h"

AnnotationDialog::ShowSelectionOnlyManager& AnnotationDialog::ShowSelectionOnlyManager::instance()
{
    static ShowSelectionOnlyManager instance;
    return instance;
}

AnnotationDialog::ShowSelectionOnlyManager::ShowSelectionOnlyManager()
    :_limit(false)
{
}

void AnnotationDialog::ShowSelectionOnlyManager::toggle()
{
    _limit = !_limit;
    if ( _limit )
        emit limitToSelected();
    else
        emit broaden();
}

void AnnotationDialog::ShowSelectionOnlyManager::unlimitFromSelection()
{
    if ( _limit ) {
        _limit = false;
        emit broaden();
    }
}


#include "ShowSelectionOnlyManager.moc"
