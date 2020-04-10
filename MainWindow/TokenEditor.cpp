/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TokenEditor.h"

#include <DB/Category.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageSearchInfo.h>
#include <Settings/SettingsData.h>

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

using namespace MainWindow;

TokenEditor::TokenEditor(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Remove Tokens"));
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);

    QWidget *mainContents = new QWidget;
    QVBoxLayout *vlay = new QVBoxLayout(mainContents);

    QLabel *label = new QLabel(i18n("Select tokens to remove from all images and videos:"));
    vlay->addWidget(label);

    QGridLayout *grid = new QGridLayout;
    vlay->addLayout(grid);

    int index = 0;
    for (int ch = 'A'; ch <= 'Z'; ch++, index++) {
        QChar token = QChar::fromLatin1((char)ch);
        QCheckBox *box = new QCheckBox(token);
        grid->addWidget(box, index / 5, index % 5);
        m_checkBoxes.append(box);
    }

    QHBoxLayout *hlay = new QHBoxLayout;
    vlay->addLayout(hlay);

    hlay->addStretch(1);
    QPushButton *selectAll = new QPushButton(i18n("Select All"));
    QPushButton *selectNone = new QPushButton(i18n("Select None"));
    hlay->addWidget(selectAll);
    hlay->addWidget(selectNone);

    connect(selectAll, &QPushButton::clicked, this, &TokenEditor::selectAll);
    connect(selectNone, &QPushButton::clicked, this, &TokenEditor::selectNone);

    dialogLayout->addWidget(mainContents);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TokenEditor::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TokenEditor::reject);
    dialogLayout->addWidget(buttonBox);
}

void TokenEditor::show()
{
    QStringList tokens = tokensInUse();

    for (QCheckBox *box : m_checkBoxes) {
        box->setChecked(false);
        QString txt = box->text().remove(QString::fromLatin1("&"));
        box->setEnabled(tokens.contains(txt));
    }
    QDialog::show();
}

void TokenEditor::selectAll()
{
    for (QCheckBox *box : m_checkBoxes) {
        box->setChecked(true);
    }
}

void TokenEditor::selectNone()
{
    for (QCheckBox *box : m_checkBoxes) {
        box->setChecked(false);
    }
}

/**
   I would love to use Settings::optionValue, but that method does not
    forget about an item once it has seen it, which is really what it should
    do anyway, otherwise it would be way to expensive in use.
*/
QStringList TokenEditor::tokensInUse()
{
    QStringList res;
    DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
    QMap<QString, DB::CountWithRange> map = DB::ImageDB::instance()->classify(DB::ImageSearchInfo(), tokensCategory->name(), DB::anyMediaType);
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        if (it.value().count > 0)
            res.append(it.key());
    }
    return res;
}

void TokenEditor::accept()
{
    DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
    for (const QCheckBox *box : m_checkBoxes) {
        if (box->isChecked() && box->isEnabled()) {
            QString txt = box->text().remove(QString::fromLatin1("&"));
            tokensCategory->removeItem(txt);
        }
    }
    QDialog::accept();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
