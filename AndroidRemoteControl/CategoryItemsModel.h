#ifndef REMOTECONTROL_CATEGORYITEMSMODEL_H
#define REMOTECONTROL_CATEGORYITEMSMODEL_H

#include <QAbstractListModel>
#include "RemoteCommand.h"

namespace RemoteControl {

using RoleMap = QHash<int, QByteArray>;
class CategoryItemsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum { TextRole, IconRole };
    explicit CategoryItemsModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    RoleMap roleNames() const override;

    void setItems(const CategoryItemsList& items);

private slots:
    void reset();

private:
    CategoryItemsList m_items;
};

} // namespace RemoteControl

Q_DECLARE_METATYPE(RemoteControl::CategoryItemsModel*);

#endif // REMOTECONTROL_CATEGORYITEMSMODEL_H
