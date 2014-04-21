#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "RemoteCommand.h"

namespace RemoteControl
{

using RoleMap = QHash<int, QByteArray>;
class CategoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum { NameRole, TextRole, IconRole, EnabledRole, TypeRole };
    explicit CategoryModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    RoleMap roleNames() const override;
    void setCategories(const QList<Category>&);

private:
    QList<Category> m_categories;
};

}
Q_DECLARE_METATYPE(RemoteControl::CategoryModel*);

#endif // CATEGORYMODEL_H
