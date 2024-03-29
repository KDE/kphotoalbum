// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl at stonemx dot de>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "MetaDataDisplay.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QMimeDatabase>
#include <QUrl>
#include <QVBoxLayout>

static QString s_noPerm = QStringLiteral("-");
static const QMimeDatabase s_mimeDB;

Exif::MetaDataDisplay::MetaDataDisplay(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *layout = new QGridLayout;
    mainLayout->addLayout(layout);

    int row = 0;

    layout->addWidget(keyLabel(i18n("Absolute path:")), row, 0);
    m_absolutePath = valueLabel();
    connect(m_absolutePath, &QLabel::linkActivated, this, &Exif::MetaDataDisplay::openDir);
    layout->addWidget(m_absolutePath, row++, 1);

    layout->addWidget(keyLabel(i18n("MIME type:")), row, 0);
    m_mimeType = valueLabel();
    layout->addWidget(m_mimeType, row++, 1);

    layout->addWidget(keyLabel(i18n("Size:")), row, 0);
    m_size = valueLabel();
    layout->addWidget(m_size, row++, 1);

    layout->addWidget(keyLabel(i18n("Created:")), row, 0);
    m_created = valueLabel();
    layout->addWidget(m_created, row++, 1);

    layout->addWidget(keyLabel(i18n("Changed:")), row, 0);
    m_modified = valueLabel();
    layout->addWidget(m_modified, row++, 1);

    layout->addWidget(keyLabel(i18n("Owner:")), row, 0);
    m_owner = valueLabel();
    layout->addWidget(m_owner, row++, 1);

    layout->addWidget(keyLabel(i18n("Group:")), row, 0);
    m_group = valueLabel();
    layout->addWidget(m_group, row++, 1);

    layout->addWidget(keyLabel(i18n("Permissions:")), row, 0);
    m_permissions = valueLabel();
    layout->addWidget(m_permissions, row++, 1);

    mainLayout->addStretch();
}

QLabel *Exif::MetaDataDisplay::keyLabel(const QString &text)
{
    auto *label = new QLabel;
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    label->setAlignment(Qt::AlignTop);
    label->setText(text);
    return label;
}

QLabel *Exif::MetaDataDisplay::valueLabel()
{
    auto *label = new QLabel;
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    return label;
}

void Exif::MetaDataDisplay::setFileName(const QString &fileName)
{
    const QFileInfo info(fileName);
    // This QLocale object should be able to be const, but due to QTBUG-71445, it can't be,
    // unless we depend on >= Qt 5.13.
    // FIXME: Make this const as soon as we depend on >= Qt 5.13
    QLocale locale;

    m_fileDir = info.absoluteDir().canonicalPath();

    m_absolutePath->setText(QStringLiteral("%1<br/><a href=\"#\">%2</a>").arg(fileName, i18n("Open folder with a file manager")));

    m_mimeType->setText(s_mimeDB.mimeTypeForFile(fileName).name());

    const auto size = info.size();
    m_size->setText(i18nc("File size composed of a pre-formatted (already localized) "
                          "human-readable file size (%1) and the number of bytes (%2)",
                          "%1 (%2 B)", locale.formattedDataSize(size), size));

    m_created->setText(locale.toString(info.birthTime()));

    m_modified->setText(locale.toString(info.lastModified()));

    m_owner->setText(info.owner());

    m_group->setText(info.group());

    const auto permissions = QFile::permissions(fileName);

    // clang-format off
    QString parsedPermissions;
    parsedPermissions.append((permissions & QFile::ReadOwner)
        ? i18nc("File permission shortcut for \"reading allowed\"", "r") : s_noPerm);
    parsedPermissions.append((permissions & QFile::WriteOwner)
        ? i18nc("File permission shortcut for \"writing allowed\"", "w") : s_noPerm);
    parsedPermissions.append((permissions & QFile::ExeOwner)
        ? i18nc("File permission shortcut for \"executing allowed\"", "x") : s_noPerm);
    parsedPermissions.append((permissions & QFile::ReadGroup)
        ? i18nc("File permission shortcut for \"reading allowed\"", "r") : s_noPerm);
    parsedPermissions.append((permissions & QFile::WriteGroup)
        ? i18nc("File permission shortcut for \"writing allowed\"", "w") : s_noPerm);
    parsedPermissions.append((permissions & QFile::ExeGroup)
        ? i18nc("File permission shortcut for \"executing allowed\"", "x") : s_noPerm);
    parsedPermissions.append((permissions & QFile::ReadOther)
        ? i18nc("File permission shortcut for \"reading allowed\"", "r") : s_noPerm);
    parsedPermissions.append((permissions & QFile::WriteOther)
        ? i18nc("File permission shortcut for \"writing allowed\"", "w") : s_noPerm);
    parsedPermissions.append((permissions & QFile::ExeOther)
        ? i18nc("File permission shortcut for \"executing allowed\"", "x") : s_noPerm);

    auto hex = 0x0000;
    if (permissions & QFile::ReadOwner)  { hex |= QFile::ReadOwner;  }
    if (permissions & QFile::WriteOwner) { hex |= QFile::WriteOwner; }
    if (permissions & QFile::ExeOwner)   { hex |= QFile::ExeOwner;   }
    if (permissions & QFile::ReadGroup)  { hex |= QFile::ReadGroup;  }
    if (permissions & QFile::WriteGroup) { hex |= QFile::WriteGroup; }
    if (permissions & QFile::ExeGroup)   { hex |= QFile::ExeGroup;   }
    if (permissions & QFile::ReadOther)  { hex |= QFile::ReadOther;  }
    if (permissions & QFile::WriteOther) { hex |= QFile::WriteOther; }
    if (permissions & QFile::ExeOther)   { hex |= QFile::ExeOther;   }
    // clang-format on

    auto octalPermissions = QString::number(hex, 16);
    octalPermissions = octalPermissions.remove(1, 1).prepend(QStringLiteral("0"));

    m_permissions->setText(i18nc("File permissions string compiled from a parsed variant (e.g. "
                                 "\"rw-rw-r--\", %1) and its octal representation (e.g. \"0644\")",
                                 "%1 (%2)", parsedPermissions, octalPermissions));
}

void Exif::MetaDataDisplay::openDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_fileDir));
}

#include "moc_MetaDataDisplay.cpp"
