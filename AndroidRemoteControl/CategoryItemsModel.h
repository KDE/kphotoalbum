#ifndef REMOTECONTROL_CATEGORYITEMSMODEL_H
#define REMOTECONTROL_CATEGORYITEMSMODEL_H

#include <QAbstractListModel>
#include "RemoteCommand.h"

namespace RemoteControl {

class CategoryItemsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum { TextRole, IconRole };
    explicit CategoryItemsModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const CategoryItemsList& items);

private:
    CategoryItemsList m_items;
};

} // namespace RemoteControl

Q_DECLARE_METATYPE(RemoteControl::CategoryItemsModel*);

#endif // REMOTECONTROL_CATEGORYITEMSMODEL_H
