#ifndef REMOTECONTROL_IMAGEDETAILS_H
#define REMOTECONTROL_IMAGEDETAILS_H

#include <QObject>
#include "RemoteCommand.h"
namespace RemoteControl {

class ImageDetails : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName MEMBER m_fileName NOTIFY updated)
    Q_PROPERTY(QString date MEMBER m_date NOTIFY updated)
    Q_PROPERTY(QString description MEMBER m_description NOTIFY updated)
    Q_PROPERTY(QStringList categories READ categories NOTIFY updated)
public:
    static ImageDetails& instance();

    QStringList categories() const;

public slots:
    void clear();
    void setData(const ImageDetailsCommand& data);
    QStringList itemsOfCategory(const QString category);

signals:
    void updated();

private:
    ImageDetails() = default;
    QString m_fileName;
    QString m_date;
    QString m_description;
    QMap<QString,QStringList> m_categories;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGEDETAILS_H
