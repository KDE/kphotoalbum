/* Copyright (C) 2019 The KPhotoAlbum Development Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e. V. (or its successor approved
 *  by the membership of KDE e. V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "PurposeMenu.h"

#include "Logging.h"

#include <MainWindow/Window.h>

#include <KLocalizedString>
#include <Purpose/AlternativesModel>
#include <PurposeWidgets/Menu>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>

Plugins::PurposeMenu::PurposeMenu(QMenu *parent)
    : QObject(parent)
    , m_parentMenu(parent)
    , m_purposeMenu(new Purpose::Menu(parent))
    , m_menuUpdateNeeded(true)
{
    loadPurposeMenu();
}

void Plugins::PurposeMenu::slotSelectionChanged()
{
    m_menuUpdateNeeded = true;
    m_purposeMenu->clear();
    qCDebug(PluginsLog) << "Purpose menu items invalidated...";
}

void Plugins::PurposeMenu::loadPurposeMenu()
{
    // attach the menu
    QAction *purposeMenu = m_parentMenu->addMenu(m_purposeMenu);
    purposeMenu->setText(i18n("Share"));
    purposeMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-share")));

    // set up the callback signal
    connect(m_purposeMenu, &Purpose::Menu::finished, this, [this](const QJsonObject &output, int error, const QString &message) {
        if (error) {
            qCDebug(PluginsLog) << "Failed to share image:" << message;
            emit imageSharingFailed(message);
        } else {
            // Note: most plugins don't seem to actually return anything in the url field...
            const QUrl returnUrl = QUrl(output[QStringLiteral("url")].toString(), QUrl::ParsingMode::StrictMode);
            qCDebug(PluginsLog) << "Image shared successfully.";
            qCDebug(PluginsLog) << "Raw json data: " << output;
            emit imageShared(returnUrl);
        }
    });

    // update available options based on the latest picture
    connect(m_purposeMenu, &QMenu::aboutToShow, this, &PurposeMenu::loadPurposeItems);
    qCDebug(PluginsLog) << "Purpose menu loaded...";
}

void Plugins::PurposeMenu::loadPurposeItems()
{
    if (!m_menuUpdateNeeded) {
        return;
    }
    m_menuUpdateNeeded = false;

    const DB::FileNameList images = MainWindow::Window::theMainWindow()->selected(ThumbnailView::NoExpandCollapsedStacks);
    QJsonArray urls;
    for (const auto &image : images) {
        urls.append(QUrl(image).toString());
    }

    // "image/jpeg" is certainly not always true, but the interface does not allow a mimeType list
    // and the plugins likely won't care...
    m_purposeMenu->model()->setInputData(QJsonObject {
        { QStringLiteral("mimeType"), QStringLiteral("image/jpeg") },
        { QStringLiteral("urls"), urls } });
    m_purposeMenu->model()->setPluginType(QStringLiteral("Export"));
    m_purposeMenu->reload();
    qCDebug(PluginsLog) << "Purpose menu items loaded...";
}
