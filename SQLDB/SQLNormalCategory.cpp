#include "SQLNormalCategory.h"

void SQLDB::SQLNormalCategory::setName(const QString& name)
{
    _qh->changeCategoryName(_categoryId, name);
    emit changed();
}

#include "SQLNormalCategory.moc"
