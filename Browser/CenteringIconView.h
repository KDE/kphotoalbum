/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CENTERINGICONVIEW_H
#define CENTERINGICONVIEW_H
#include <QListView>
class QTimer;

namespace Browser
{

/**
 * \brief QListView subclass which shows its content as icons centered in the middle
 *
 * To make KPhotoAlbum slightly more sexy, the overview page has its items
 * centered at the middle of the screen. This class has the capability to
 * do that when \ref setViewMode is called with \ref CenterView as argument.
 */
class CenteringIconView : public QListView
{
    Q_OBJECT
public:
    enum ViewMode { CenterView,
                    NormalIconView };

    explicit CenteringIconView(QWidget *parent);
    void setViewMode(ViewMode);
    void setModel(QAbstractItemModel *) override;
    void showEvent(QShowEvent *) override;

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void setupMargins();
    int columnCount(int elementCount) const;
    int availableWidth() const;
    int availableHeight() const;

private:
    ViewMode m_viewMode;
};

}

#endif /* CENTERINGICONVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
