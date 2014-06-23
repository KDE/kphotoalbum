#ifndef TAGGEDAREA_H
#define TAGGEDAREA_H

#include <QFrame>

namespace Viewer {

class TaggedArea : public QFrame
{
    Q_OBJECT

public:
    explicit TaggedArea(QWidget *parent = 0);
    ~TaggedArea();
    void setTagInfo(QString category, QString localizedCategory, QString tag);
    void setActualGeometry(QRect geometry);
    QRect actualGeometry() const;

public slots:
    void checkShowArea(QPair<QString, QString> tagData);
    void resetViewStyle();

private:
    QPair<QString, QString> _tagInfo;
    QRect _actualGeometry;
    QString _styleDefault;
    QString _styleHighlighted;
};

}

#endif // TAGGEDAREA_H
// vi:expandtab:tabstop=4 shiftwidth=4:
