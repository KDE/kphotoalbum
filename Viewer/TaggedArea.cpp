#include "TaggedArea.h"
#include <klocale.h>
#include <QDebug>
#include <QApplication>

Viewer::TaggedArea::TaggedArea(QWidget *parent) : QFrame(parent)
{
    setFrameShape(QFrame::Box);
    resetViewStyle();
}

Viewer::TaggedArea::~TaggedArea()
{
}

void Viewer::TaggedArea::setTagInfo(QString category, QString localizedCategory, QString tag)
{
    setToolTip(tag + QString::fromLatin1(" (") + localizedCategory + QString::fromLatin1(")"));
    _tagInfo = QPair<QString, QString>(category, tag);
}

void Viewer::TaggedArea::setActualGeometry(QRect geometry)
{
    _actualGeometry = geometry;
}

QRect Viewer::TaggedArea::actualGeometry() const
{
    return _actualGeometry;
}

void Viewer::TaggedArea::resetViewStyle()
{
    setStyleSheet(QString::fromLatin1(
        "Viewer--TaggedArea { border: none; background-color: none; }"
        "Viewer--TaggedArea:hover { border: 1px solid rgb(0,255,0,99); background-color: rgb(255,255,255,30); }"
    ));
}

void Viewer::TaggedArea::checkShowArea(QPair<QString, QString> tagData)
{
    if (tagData == _tagInfo) {
        setStyleSheet(QString::fromLatin1("Viewer--TaggedArea { border: 1px solid rgb(0,255,0,99); background-color: rgb(255,255,255,30); }"));
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
