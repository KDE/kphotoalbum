// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "InfoDialog.h"

#include "Grid.h"
#include "MetaDataDisplay.h"
#include <kpaexif/Info.h>

#include <DB/ImageDB.h>
#include <ImageManager/AsyncLoader.h>
#include <ImageManager/ImageRequest.h>
#include <kpabase/SettingsData.h>

#include <KLocalizedString>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTextCodec>

using Utilities::StringSet;

Exif::InfoDialog::InfoDialog(const DB::FileName &fileName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Exif Information and file metadata"));

    setAttribute(Qt::WA_DeleteOnClose);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Close)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QWidget *top = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(top);
    setLayout(vlay);
    vlay->addWidget(top);

    // -------------------------------------------------- File name and pixmap
    QHBoxLayout *hlay = new QHBoxLayout;
    vlay->addLayout(hlay);
    m_fileNameLabel = new QLabel(top);
    QFont fnt = font();
    fnt.setPointSize((int)(fnt.pointSize() * 1.2));
    fnt.setWeight(QFont::Bold);
    m_fileNameLabel->setFont(fnt);
    m_fileNameLabel->setAlignment(Qt::AlignCenter);
    hlay->addWidget(m_fileNameLabel, 1);

    m_pix = new QLabel(top);
    hlay->addWidget(m_pix);

    // -------------------------------------------------- Exif info part

    auto *exifWidget = new QWidget;
    auto *exifWidgetLayout = new QVBoxLayout(exifWidget);

    // -------------------------------------------------- Exif Grid
    m_grid = new Exif::Grid(top);
    exifWidgetLayout->addWidget(m_grid);

    // -------------------------------------------------- Current Search
    hlay = new QHBoxLayout;
    exifWidgetLayout->addLayout(hlay);

    m_searchBox = new QLineEdit(top);
    m_searchBox->setPlaceholderText(i18nc("@label:textbox The search box allows the user to filter by exif label names", "Filter labels ..."));
    hlay->addWidget(m_searchBox);
    hlay->addStretch(1);

    QLabel *iptcLabel = new QLabel(i18n("IPTC character set:"), top);
    m_iptcCharset = new QComboBox(top);
    QStringList charsets;
    QList<QByteArray> charsetsBA = QTextCodec::availableCodecs();
    for (QList<QByteArray>::const_iterator it = charsetsBA.constBegin(); it != charsetsBA.constEnd(); ++it)
        charsets << QLatin1String(*it);
    m_iptcCharset->insertItems(0, charsets);
    m_iptcCharset->setCurrentIndex(qMax(0, QTextCodec::availableCodecs().indexOf(Settings::SettingsData::instance()->iptcCharset().toLatin1())));
    hlay->addWidget(iptcLabel);
    hlay->addWidget(m_iptcCharset);

    // -------------------------------------------------- File metadata part

    m_metaDataDisplay = new MetaDataDisplay;

    // -------------------------------------------------- Tab widget
    auto *tabWidget = new QTabWidget;
    vlay->addWidget(tabWidget);
    tabWidget->addTab(exifWidget, i18n("Exif info"));
    tabWidget->addTab(m_metaDataDisplay, i18n("File metadata"));

    // -------------------------------------------------- layout done
    connect(m_searchBox, &QLineEdit::textChanged, m_grid, &Grid::updateSearchString);
    connect(m_iptcCharset, &QComboBox::textActivated, m_grid, &Grid::setupUI);
    setImage(fileName);

    vlay->addWidget(buttonBox);
}

QSize Exif::InfoDialog::sizeHint() const
{
    return QSize(800, 400);
}

void Exif::InfoDialog::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    if (request->loadedOK())
        m_pix->setPixmap(QPixmap::fromImage(image));
}

void Exif::InfoDialog::setImage(const DB::FileName &fileName)
{
    m_fileNameLabel->setText(fileName.relative());
    m_grid->setFileName(fileName);
    m_metaDataDisplay->setFileName(fileName.absolute());

    const auto info = DB::ImageDB::instance()->info(fileName);
    ImageManager::ImageRequest *request = new ImageManager::ImageRequest(fileName, QSize(128, 128), info->angle(), this);
    request->setPriority(ImageManager::Viewer);
    ImageManager::AsyncLoader::instance()->load(request);
}

void Exif::InfoDialog::enterEvent(QEnterEvent *)
{
    m_grid->setFocus();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_InfoDialog.cpp"
