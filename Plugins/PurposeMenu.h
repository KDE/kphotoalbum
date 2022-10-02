// SPDX-FileCopyrightText: 2019-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef KPHOTOALBUM_PURPOSEMENU_H
#define KPHOTOALBUM_PURPOSEMENU_H

#include <kpabase/config-kpa-plugins.h>

#include <PurposeWidgets/Menu>
#include <QString>
#include <QUrl>

class QMenu;

namespace Purpose
{
class Menu;
}

namespace Plugins
{

class PurposeMenu : public Purpose::Menu
{
    Q_OBJECT
public:
    explicit PurposeMenu(QMenu *parent);

public Q_SLOTS:
    void slotSelectionChanged();

Q_SIGNALS:
    /**
     * @brief imageShared is emitted when an image was shared successfully.
     * The url contains the optional location of the shared data
     * (e.g. for plugins that upload to a remote location).
     */
    void imageShared(QUrl);
    void imageSharingFailed(QString message);

private:
    QMenu *m_parentMenu;
    bool m_menuUpdateNeeded; ///< Keeps track of changed image selection
    /**
     * @brief Load the Purpose::Menu, add it to the parent menu, and set up connections.
     */
    void loadPurposeMenu();
    /**
     * @brief Load Purpose menu items into the Purpose::Menu.
     * This is dependent on the current set of images.
     */
    void loadPurposeItems();
};

}

#endif /* PURPOSEMENU_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
