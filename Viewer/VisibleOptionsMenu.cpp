// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VisibleOptionsMenu.h"

#include <DB/Category.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <kpabase/SettingsData.h>

#include <KActionCollection>
#include <KLocalizedString>
#include <KToggleAction>
#include <QCheckBox>
#include <QDebug>
#include <QList>

#include <utility>

Viewer::VisibleOptionsMenu::VisibleOptionsMenu(QWidget *parent, KActionCollection *actions)
    : QMenu(i18n("Show..."), parent)
{
    setTearOffEnabled(true);
    setTitle(i18n("Show"));
    connect(this, &VisibleOptionsMenu::aboutToShow, this, &VisibleOptionsMenu::updateState);

    m_showInfoBox = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-infobox"));
    m_showInfoBox->setText(i18n("Show Info Box"));
    actions->setDefaultShortcut(m_showInfoBox, Qt::CTRL + Qt::Key_I);
    m_showInfoBox->setChecked(Settings::SettingsData::instance()->showInfoBox());
    connect(m_showInfoBox, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowInfoBox);
    addAction(m_showInfoBox);

    m_showLabel = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-label"));
    m_showLabel->setText(i18n("Show Label"));
    connect(m_showLabel, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowLabel);
    addAction(m_showLabel);

    m_showDescription = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-description"));
    m_showDescription->setText(i18n("Show Description"));
    connect(m_showDescription, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowDescription);
    addAction(m_showDescription);

    m_showDate = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-date"));
    m_showDate->setText(i18n("Show Date"));
    connect(m_showDate, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowDate);
    addAction(m_showDate);

    m_showTime = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-time"));
    m_showTime->setText(i18n("Show Time"));
    connect(m_showTime, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowTime);
    addAction(m_showTime);
    m_showTime->setVisible(m_showDate->isChecked());

    m_showFileName = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-filename"));
    m_showFileName->setText(i18n("Show Filename"));
    connect(m_showFileName, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowFilename);
    addAction(m_showFileName);

    m_showExif = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-exif"));
    m_showExif->setText(i18n("Show Exif"));
    connect(m_showExif, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowEXIF);
    addAction(m_showExif);

    m_showImageSize = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-imagesize"));
    m_showImageSize->setText(i18n("Show Image Size"));
    connect(m_showImageSize, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowImageSize);
    addAction(m_showImageSize);

    m_showRating = actions->add<KToggleAction>(QString::fromLatin1("viewer-show-rating"));
    m_showRating->setText(i18n("Show Rating"));
    connect(m_showRating, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowRating);
    addAction(m_showRating);

    const QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const auto &category : categories) {
        KToggleAction *taction = actions->add<KToggleAction>(category->name());
        m_actionList.append(taction);
        taction->setText(category->name());
        taction->setData(category->name());
        addAction(taction);
        connect(taction, &KToggleAction::toggled, this, &VisibleOptionsMenu::toggleShowCategory);
    }
}

void Viewer::VisibleOptionsMenu::toggleShowCategory(bool b)
{
    QAction *action = qobject_cast<QAction *>(sender());
    DB::ImageDB::instance()->categoryCollection()->categoryForName(action->data().value<QString>())->setDoShow(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowLabel(bool b)
{
    Settings::SettingsData::instance()->setShowLabel(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowDescription(bool b)
{
    Settings::SettingsData::instance()->setShowDescription(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowDate(bool b)
{
    Settings::SettingsData::instance()->setShowDate(b);
    m_showTime->setVisible(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowFilename(bool b)
{
    Settings::SettingsData::instance()->setShowFilename(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowTime(bool b)
{
    Settings::SettingsData::instance()->setShowTime(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowEXIF(bool b)
{
    Settings::SettingsData::instance()->setShowEXIF(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowImageSize(bool b)
{
    Settings::SettingsData::instance()->setShowImageSize(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowRating(bool b)
{
    Settings::SettingsData::instance()->setShowRating(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::toggleShowInfoBox(bool b)
{
    Settings::SettingsData::instance()->setShowInfoBox(b);
    Q_EMIT visibleOptionsChanged();
}

void Viewer::VisibleOptionsMenu::updateState()
{
    m_showInfoBox->setChecked(Settings::SettingsData::instance()->showInfoBox());
    m_showLabel->setChecked(Settings::SettingsData::instance()->showLabel());
    m_showDescription->setChecked(Settings::SettingsData::instance()->showDescription());
    m_showDate->setChecked(Settings::SettingsData::instance()->showDate());
    m_showTime->setChecked(Settings::SettingsData::instance()->showTime());
    m_showFileName->setChecked(Settings::SettingsData::instance()->showFilename());
    m_showExif->setChecked(Settings::SettingsData::instance()->showEXIF());
    m_showImageSize->setChecked(Settings::SettingsData::instance()->showImageSize());
    m_showRating->setChecked(Settings::SettingsData::instance()->showRating());

    const auto categoryCollection = DB::ImageDB::instance()->categoryCollection();
    for (KToggleAction *action : std::as_const(m_actionList)) {
        const auto category = categoryCollection->categoryForName(action->data().value<QString>());
        action->setChecked(category->doShow());
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VisibleOptionsMenu.cpp"
