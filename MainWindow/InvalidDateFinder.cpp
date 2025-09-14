// SPDX-FileCopyrightText: 2003 - 2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "InvalidDateFinder.h"

#include "Window.h"

#include <DB/FileInfo.h>
#include <DB/ImageDB.h>
#include <DB/ImageDate.h>
#include <DB/ImageInfo.h>
#include <Utilities/ShowBusyCursor.h>

#include <KLocalizedString>
#include <KTextEdit>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <qapplication.h>
#include <qeventloop.h>
#include <qlayout.h>
#include <qradiobutton.h>

using namespace MainWindow;

InvalidDateFinder::InvalidDateFinder(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Search for Images and Videos with Missing Dates"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QGroupBox *grp = new QGroupBox(i18n("Which Images and Videos to Display"));
    QVBoxLayout *grpLay = new QVBoxLayout(grp);
    mainLayout->addWidget(grp);

    m_dateNotTime = new QRadioButton(i18n("Search for images and videos with a valid date but an invalid time stamp"));
    m_missingDate = new QRadioButton(i18n("Search for images and videos missing date and time"));
    m_partialDate = new QRadioButton(i18n("Search for images and videos with only partial dates (like 1971 vs. 11/7-1971)"));
    m_dateNotTime->setChecked(true);

    grpLay->addWidget(m_dateNotTime);
    grpLay->addWidget(m_missingDate);
    grpLay->addWidget(m_partialDate);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &InvalidDateFinder::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &InvalidDateFinder::reject);
    mainLayout->addWidget(buttonBox);
}

void InvalidDateFinder::accept()
{
    QDialog::accept();
    Utilities::ShowBusyCursor dummy;

    // create the info dialog
    QDialog *info = new QDialog;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    info->setLayout(mainLayout);
    info->setWindowTitle(i18nc("@title:window", "Image Info"));

    KTextEdit *edit = new KTextEdit(info);
    mainLayout->addWidget(edit);
    edit->setText(i18n("<h1>Here you may see the date changes for the displayed items.</h1>"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    info->connect(buttonBox, &QDialogButtonBox::accepted, info, &QDialog::accept);
    info->connect(buttonBox, &QDialogButtonBox::rejected, info, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Now search for the images.
    const auto images = DB::ImageDB::instance()->images();
    DB::FileNameList toBeShown;
    QProgressDialog dialog(nullptr);
    dialog.setWindowTitle(i18nc("@title:window", "Reading File Properties"));
    dialog.setMaximum(images.size());
    dialog.setValue(0);
    int progress = 0;

    for (const auto &info : images) {
        dialog.setValue(++progress);
        qApp->processEvents(QEventLoop::AllEvents);
        if (dialog.wasCanceled())
            break;
        if (info->isNull())
            continue;

        // This is the date/time stored in the XML database file.
        const DB::ImageDate xmlDateTime = info->date();
        bool show = false;
        if (m_dateNotTime->isChecked()) {
            // Read the date/time stored in the image file's header (if any) or the
            // file's modification time otherwise (if this option is enabled in
            // the settings).
            DB::FileInfo fi { info->fileName(), DB::EXIFMODE_DATE };
            const Utilities::FastDateTime imageDateTime = fi.dateTime();
            if (imageDateTime.date() == xmlDateTime.start().date()) {
                show = (imageDateTime.time() != xmlDateTime.start().time());
            }
            if (show) {
                edit->append(QString::fromLatin1("%1:<br/>XML date = %2<br>File date = %3").arg(info->fileName().relative(), xmlDateTime.start().toString(), imageDateTime.toString()));
            }
        } else if (m_missingDate->isChecked()) {
            show = !xmlDateTime.start().isValid();
        } else if (m_partialDate->isChecked()) {
            show = (xmlDateTime.start() != xmlDateTime.end());
        }

        if (show)
            toBeShown.append(info->fileName());
    }

    if (m_dateNotTime->isChecked()) {
        info->resize(800, 600);
        edit->setReadOnly(true);
        QFont f = edit->font();
        f.setFamily(QString::fromLatin1("fixed"));
        edit->setFont(f);
        info->show();
    } else
        delete info;

    Window::theMainWindow()->showThumbNails(toBeShown);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_InvalidDateFinder.cpp"
