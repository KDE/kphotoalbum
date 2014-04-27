#ifndef REMOTECONTROL_DISCOVERYMODEL_H
#define REMOTECONTROL_DISCOVERYMODEL_H

#include "ThumbnailModel.h"

namespace RemoteControl {

class DiscoveryModel : public ThumbnailModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

public:
    DiscoveryModel(QObject* parent);
    int count() const;
    void setImages(const QList<int>&images) override;

public slots:
    void setCount(int arg);
    void resetImages();

signals:
    void countChanged();

private:
    int m_count = 0;
    QList<int> m_allImages;
};

} // namespace RemoteControl

Q_DECLARE_METATYPE(RemoteControl::DiscoveryModel*);

#endif // REMOTECONTROL_DISCOVERYMODEL_H
