// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BREADCRUMBVIEWER_H
#define BREADCRUMBVIEWER_H
#include <Browser/BreadcrumbList.h>

#include <QLabel>

namespace MainWindow
{
class BreadcrumbViewer : public QLabel
{
    Q_OBJECT

public:
    BreadcrumbViewer();
    QSize minimumSizeHint() const override;

public Q_SLOTS:
    void setBreadcrumbs(const Browser::BreadcrumbList &list);

Q_SIGNALS:
    void widenToBreadcrumb(const Browser::Breadcrumb &);

protected:
    void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
    void linkClicked(const QString &link);

private:
    void updateText();

private:
    Browser::BreadcrumbList m_activeCrumbs;
};
}

#endif /* BREADCRUMBVIEWER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
