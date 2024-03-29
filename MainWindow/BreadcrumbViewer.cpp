// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "BreadcrumbViewer.h"

#include <QTextDocument>

void MainWindow::BreadcrumbViewer::setBreadcrumbs(const Browser::BreadcrumbList &list)
{
    m_activeCrumbs = list.latest();
    updateText();
}

void MainWindow::BreadcrumbViewer::linkClicked(const QString &link)
{
    Q_EMIT widenToBreadcrumb(m_activeCrumbs[link.toInt()]);
}

MainWindow::BreadcrumbViewer::BreadcrumbViewer()
{
    connect(this, &BreadcrumbViewer::linkActivated, this, &BreadcrumbViewer::linkClicked);
}

/**
 * Format the text with hyperlinks. The tricky part is to handle the situation where all the text doesn't fit in.
 * The by far best solution would be to compress at a letter level, but this code is really only used in the rare
 * situation where the user chooses a very long path, as his window usually is somewhat wide.
 */
void MainWindow::BreadcrumbViewer::updateText()
{
    if (m_activeCrumbs.isEmpty())
        return;

    QStringList htmlList;

    for (int i = 0; i < m_activeCrumbs.count() - 1; ++i)
        htmlList.append(QString::fromLatin1("<a href=\"%1\">%2</a>").arg(i).arg(m_activeCrumbs[i].text()));
    if (!m_activeCrumbs[m_activeCrumbs.count() - 1].isView())
        htmlList.append(m_activeCrumbs[m_activeCrumbs.count() - 1].text());

    QTextDocument doc;
    doc.setDefaultFont(font());

    QString res = htmlList.last();
    const QString ellipses = QChar(0x2026) + QString::fromLatin1(" > ");
    for (int i = htmlList.count() - 2; i >= 0; --i) {
        // If we can't fit it in, then add ellipses
        const QString tmp = htmlList[i] + QString::fromLatin1(" > ") + res;
        doc.setHtml(tmp);
        if (doc.size().width() > width()) {
            res = ellipses + res;
            break;
        }

        // now check that we can fit in ellipses if this was the last token
        const QString tmp2 = ellipses + tmp;
        doc.setHtml(tmp2);
        if (doc.size().width() > width() && i != 0) {
            // Nope, so better stop here
            res = ellipses + res;
            break;
        }

        res = tmp;
    }

    setText(res);
}

void MainWindow::BreadcrumbViewer::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updateText();
}

QSize MainWindow::BreadcrumbViewer::minimumSizeHint() const
{
    return QSize(100, QLabel::minimumSizeHint().height());
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_BreadcrumbViewer.cpp"
