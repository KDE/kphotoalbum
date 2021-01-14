/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "Grid.h"

#include "Info.h"

#include <kpabase/SettingsData.h>

#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>

Exif::Grid::Grid(QWidget *parent)
    : QScrollArea(parent)
{
    setFocusPolicy(Qt::WheelFocus);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    viewport()->installEventFilter(this);
    setMinimumSize(800, 400);
}

class Background : public QWidget
{
protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        painter.fillRect(event->rect(), QColor(palette().window().color()));
    }
};

void Exif::Grid::setupUI(const QString &charset)
{
    delete this->widget();
    m_labels.clear();
    Background *widget = new Background;

    QGridLayout *layout = new QGridLayout(widget);
    layout->setSpacing(0);
    int row = 0;

    const QMap<QString, QStringList> map = Exif::Info::instance()->infoForDialog(m_fileName, charset);
    const StringSet groups = exifGroups(map);

    for (const QString &group : groups) {
        layout->addWidget(headerLabel(group), row++, 0, 1, 4);

        // Items of group
        const QMap<QString, QStringList> items = itemsForGroup(group, map);
        QStringList sorted = items.keys();
        sorted.sort();
        int elements = sorted.size();
        int perCol = (elements + 1) / 2;
        int count = 0;
        for (const QString &key : sorted) {
            const int subrow = (count % perCol);
            const QPalette::ColorRole role = (subrow & 1) ? QPalette::Base : QPalette::AlternateBase;
            QPair<QLabel *, QLabel *> pair = infoLabelPair(exifNameNoGroup(key), items[key].join(QLatin1String(", ")), role);

            int col = (count / perCol) * 2;
            layout->addWidget(pair.first, row + subrow, col);
            layout->addWidget(pair.second, row + subrow, col + 1);
            count++;
        }
        row += perCol;
    }

    setWidget(widget);
    widget->show();
    QTimer::singleShot(0, this, SLOT(updateWidgetSize()));
}

QLabel *Exif::Grid::headerLabel(const QString &title)
{
    QLabel *label = new QLabel(title);

    label->setBackgroundRole(QPalette::Dark);
    label->setForegroundRole(QPalette::BrightText);
    label->setAutoFillBackground(true);
    label->setAlignment(Qt::AlignCenter);

    return label;
}

QPair<QLabel *, QLabel *> Exif::Grid::infoLabelPair(const QString &title, const QString &value, const QPalette::ColorRole backgroundRole)
{
    QLabel *keyLabel = new QLabel(title);
    QLabel *valueLabel = new QLabel(value);

    keyLabel->setBackgroundRole(backgroundRole);
    valueLabel->setBackgroundRole(backgroundRole);
    keyLabel->setAutoFillBackground(true);
    valueLabel->setAutoFillBackground(true);
    m_labels.append(qMakePair(keyLabel, valueLabel));
    return qMakePair(keyLabel, valueLabel);
}

void Exif::Grid::updateWidgetSize()
{
    widget()->setFixedSize(viewport()->width(), widget()->height());
}

StringSet Exif::Grid::exifGroups(const QMap<QString, QStringList> &exifInfo)
{
    StringSet result;
    for (QMap<QString, QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it) {
        result.insert(groupName(it.key()));
    }
    return result;
}

QMap<QString, QStringList> Exif::Grid::itemsForGroup(const QString &group, const QMap<QString, QStringList> &exifInfo)
{
    QMap<QString, QStringList> result;
    for (QMap<QString, QStringList>::ConstIterator it = exifInfo.begin(); it != exifInfo.end(); ++it) {
        if (groupName(it.key()) == group)
            result.insert(it.key(), it.value());
    }
    return result;
}

QString Exif::Grid::groupName(const QString &exifName)
{
    QStringList list = exifName.split(QString::fromLatin1("."));
    list.pop_back();
    return list.join(QString::fromLatin1("."));
}

QString Exif::Grid::exifNameNoGroup(const QString &fullName)
{
    return fullName.split(QString::fromLatin1(".")).last();
}

void Exif::Grid::scroll(int dy)
{
    verticalScrollBar()->setValue(verticalScrollBar()->value() + dy);
}

void Exif::Grid::updateSearchString(const QString &search)
{
    for (QPair<QLabel *, QLabel *> tuple : m_labels) {
        const bool matches = tuple.first->text().contains(search, Qt::CaseInsensitive) && search.length() != 0;
        const auto fgRole = matches ? QPalette::HighlightedText : QPalette::Text;
        const auto bgRole = matches ? QPalette::Highlight : QPalette::Base;
        tuple.first->setForegroundRole(fgRole);
        tuple.first->setBackgroundRole(bgRole);
        tuple.second->setForegroundRole(fgRole);
        tuple.second->setBackgroundRole(bgRole);
        QFont fnt = tuple.first->font();
        fnt.setBold(matches);
        tuple.first->setFont(fnt);
        tuple.second->setFont(fnt);
    }
}

void Exif::Grid::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Down:
        scroll(20);
        return;
    case Qt::Key_Up:
        scroll(-20);
        return;
    case Qt::Key_PageDown:
        scroll(viewport()->height() - 20);
        return;
    case Qt::Key_PageUp:
        scroll(-(viewport()->height() - 20));
        return;
    case Qt::Key_Escape:
        QScrollArea::keyPressEvent(e); // Propagate to close dialog.
        return;
    }
}

bool Exif::Grid::eventFilter(QObject *object, QEvent *event)
{
    if (object == viewport() && event->type() == QEvent::Resize) {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        widget()->setFixedSize(re->size().width(), widget()->height());
    }
    return false;
}

void Exif::Grid::setFileName(const DB::FileName &fileName)
{
    m_fileName = fileName;
    setupUI(Settings::SettingsData::instance()->iptcCharset());
}
// vi:expandtab:tabstop=4 shiftwidth=4:
