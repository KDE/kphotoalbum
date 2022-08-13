/* SPDX-FileCopyrightText: 2019-2020 The KPhotoAlbum Development Team
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    : Purpose::Menu(parent)
    , m_parentMenu(parent)
    , m_menuUpdateNeeded(true)
{
    loadPurposeMenu();
}

void Plugins::PurposeMenu::slotSelectionChanged()
{
    m_menuUpdateNeeded = true;
    clear();
    qCDebug(PluginsLog) << "Purpose menu items invalidated...";
}

void Plugins::PurposeMenu::loadPurposeMenu()
{
    // attach the menu
    QAction *purposeMenu = m_parentMenu->addMenu(this);
    purposeMenu->setText(i18n("Share"));
    purposeMenu->setIcon(QIcon::fromTheme(QStringLiteral("document-share")));

    // set up the callback signal
    connect(this, &Purpose::Menu::finished, this, [this](const QJsonObject &output, int error, const QString &message) {
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
    connect(this, &QMenu::aboutToShow, this, &PurposeMenu::loadPurposeItems);
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
    model()->setInputData(QJsonObject {
        { QStringLiteral("mimeType"), QStringLiteral("image/jpeg") },
        { QStringLiteral("urls"), urls } });
    model()->setPluginType(QStringLiteral("Export"));
    reload();
    qCDebug(PluginsLog) << "Purpose menu items loaded...";
}

#include "moc_PurposeMenu.cpp"
