// SPDX-FileCopyrightText: 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2010-2013 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012 Frederik Schwarzer <schwarzer@kde.org>
// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2018 Antoni Bella Pérez <antonibella5@yahoo.com>
// SPDX-FileCopyrightText: 2018-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AutoStackImages.h"

#include "Window.h"

#include <DB/FileInfo.h>
#include <DB/ImageDB.h>
#include <DB/ImageDate.h>
#include <DB/ImageInfo.h>
#include <DB/MD5Map.h>
#include <Utilities/ShowBusyCursor.h>
#include <kpabase/FileUtil.h>
#include <kpabase/SettingsData.h>

#include <KLocalizedString>
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QRegularExpression>

#include <utility>

using namespace MainWindow;

AutoStackImages::AutoStackImages(QWidget *parent, const DB::FileNameList &list)
    : QDialog(parent)
    , m_list(list)
{
    setWindowTitle(i18nc("@title:window", "Automatically Stack Images"));

    QWidget *top = new QWidget;
    QVBoxLayout *lay1 = new QVBoxLayout(top);
    setLayout(lay1);

    QWidget *containerMd5 = new QWidget(this);
    lay1->addWidget(containerMd5);
    QHBoxLayout *hlayMd5 = new QHBoxLayout(containerMd5);

    m_matchingMD5 = new QCheckBox(i18n("Stack images with identical MD5 sum"));
    m_matchingMD5->setChecked(false);
    hlayMd5->addWidget(m_matchingMD5);

    QWidget *containerFile = new QWidget(this);
    lay1->addWidget(containerFile);
    QHBoxLayout *hlayFile = new QHBoxLayout(containerFile);

    m_matchingFile = new QCheckBox(i18n("Stack images based on file version detection"));
    m_matchingFile->setChecked(true);
    hlayFile->addWidget(m_matchingFile);

    m_origTop = new QCheckBox(i18n("Original to top"));
    m_origTop->setChecked(false);
    hlayFile->addWidget(m_origTop);

    QWidget *containerContinuous = new QWidget(this);
    lay1->addWidget(containerContinuous);
    QHBoxLayout *hlayContinuous = new QHBoxLayout(containerContinuous);

    // Would minutes not be a more sane time unit here? (schwarzer)
    // jzarl: this feature is used to group "burst" images - usually they are taken within a few seconds.
    // a typical value would be somewhere between 2 and 20 seconds
    m_continuousShooting = new QCheckBox(i18nc("Label for a QSpinbox that selects a number of seconds.", "Stack images created within the following time-frame:"));
    m_continuousShooting->setChecked(false);
    hlayContinuous->addWidget(m_continuousShooting);

    m_continuousThreshold = new QSpinBox;
    m_continuousThreshold->setRange(1, 999);
    m_continuousThreshold->setSingleStep(1);
    m_continuousThreshold->setValue(2);
    m_continuousThreshold->setSuffix(i18nc("Unit suffix on a QSpinBox; note the space at the beginning.", " seconds"));
    hlayContinuous->addWidget(m_continuousThreshold);

    QGroupBox *grpOptions = new QGroupBox(i18n("AutoStacking Options"));
    QVBoxLayout *grpLayOptions = new QVBoxLayout(grpOptions);
    lay1->addWidget(grpOptions);

    m_autostackDefault = new QRadioButton(i18n("Include matching image to appropriate stack (if one exists)"));
    m_autostackDefault->setChecked(true);
    grpLayOptions->addWidget(m_autostackDefault);

    m_autostackUnstack = new QRadioButton(i18n("Unstack images from their current stack and create new one for the matches"));
    m_autostackUnstack->setChecked(false);
    grpLayOptions->addWidget(m_autostackUnstack);

    m_autostackSkip = new QRadioButton(i18n("Skip images that are already in a stack"));
    m_autostackSkip->setChecked(false);
    grpLayOptions->addWidget(m_autostackSkip);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AutoStackImages::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AutoStackImages::reject);
    lay1->addWidget(buttonBox);
}

/*
 * This function searches for images with matching MD5 sums
 * Matches are automatically stacked
 */

void AutoStackImages::matchingMD5(DB::FileNameList &toBeShown)
{
    QMap<DB::MD5, DB::FileNameList> tostack;
    DB::FileNameList showIfStacked;

    // Stacking all images that have the same MD5 sum
    // First make a map of MD5 sums with corresponding images
    for (const DB::FileName &fileName : std::as_const(m_list)) {
        const auto info = DB::ImageDB::instance()->info(fileName);
        DB::MD5 sum = info->MD5Sum();
        if (DB::ImageDB::instance()->md5Map()->contains(sum)) {
            if (tostack[sum].isEmpty())
                tostack.insert(sum, DB::FileNameList() << fileName);
            else
                tostack[sum].append(fileName);
        }
    }

    // Then add images to stack (depending on configuration options)
    for (QMap<DB::MD5, DB::FileNameList>::ConstIterator it = tostack.constBegin(); it != tostack.constEnd(); ++it) {
        if (tostack[it.key()].count() > 1) {
            DB::FileNameList stack;
            for (int i = 0; i < tostack[it.key()].count(); ++i) {
                if (!DB::ImageDB::instance()->getStackFor(tostack[it.key()][i]).isEmpty()) {
                    if (m_autostackUnstack->isChecked())
                        DB::ImageDB::instance()->unstack(DB::FileNameList() << tostack[it.key()][i]);
                    else if (m_autostackSkip->isChecked())
                        continue;
                }

                showIfStacked.append(tostack[it.key()][i]);
                stack.append(tostack[it.key()][i]);
            }
            if (stack.size() > 1) {

                for (const DB::FileName &a : std::as_const(showIfStacked)) {
                    if (!DB::ImageDB::instance()->getStackFor(a).isEmpty()) {
                        const auto stackedImages = DB::ImageDB::instance()->getStackFor(a);
                        for (const DB::FileName &b : stackedImages)
                            toBeShown.append(b);
                    } else
                        toBeShown.append(a);
                }
                DB::ImageDB::instance()->stack(stack);
            }
            showIfStacked.clear();
        }
    }
}

/*
 * This function searches for images based on file version detection configuration.
 * Images that are detected to be versions of same file are stacked together.
 */

void AutoStackImages::matchingFile(DB::FileNameList &toBeShown)
{
    QMap<DB::MD5, DB::FileNameList> tostack;
    DB::FileNameList showIfStacked;
    QString modifiedFileCompString;
    QRegularExpression modifiedFileComponent;
    QStringList originalFileComponents;

    modifiedFileCompString = Settings::SettingsData::instance()->modifiedFileComponent();
    modifiedFileComponent = QRegularExpression(modifiedFileCompString);

    originalFileComponents << Settings::SettingsData::instance()->originalFileComponent();
    originalFileComponents = originalFileComponents.at(0).split(QString::fromLatin1(";"));

    // Stacking all images based on file version detection
    // First round prepares the stacking
    for (const DB::FileName &fileName : std::as_const(m_list)) {
        if (modifiedFileCompString.length() >= 0 && fileName.relative().contains(modifiedFileComponent)) {

            for (QStringList::const_iterator it = originalFileComponents.constBegin();
                 it != originalFileComponents.constEnd(); ++it) {
                QString tmp = fileName.relative();
                tmp.replace(modifiedFileComponent, (*it));
                DB::FileName originalFileName = DB::FileName::fromRelativePath(tmp);

                if (originalFileName != fileName && m_list.contains(originalFileName)) {
                    const auto info = DB::ImageDB::instance()->info(fileName);
                    DB::MD5 sum = info->MD5Sum();
                    if (tostack[sum].isEmpty()) {
                        if (m_origTop->isChecked()) {
                            tostack.insert(sum, DB::FileNameList() << originalFileName);
                            tostack[sum].append(fileName);
                        } else {
                            tostack.insert(sum, DB::FileNameList() << fileName);
                            tostack[sum].append(originalFileName);
                        }
                    } else
                        tostack[sum].append(fileName);
                    break;
                }
            }
        }
    }

    // Then add images to stack (depending on configuration options)
    for (QMap<DB::MD5, DB::FileNameList>::ConstIterator it = tostack.constBegin(); it != tostack.constEnd(); ++it) {
        if (tostack[it.key()].count() > 1) {
            DB::FileNameList stack;
            for (int i = 0; i < tostack[it.key()].count(); ++i) {
                if (!DB::ImageDB::instance()->getStackFor(tostack[it.key()][i]).isEmpty()) {
                    if (m_autostackUnstack->isChecked())
                        DB::ImageDB::instance()->unstack(DB::FileNameList() << tostack[it.key()][i]);
                    else if (m_autostackSkip->isChecked())
                        continue;
                }

                showIfStacked.append(tostack[it.key()][i]);
                stack.append(tostack[it.key()][i]);
            }
            if (stack.size() > 1) {

                for (const DB::FileName &a : std::as_const(showIfStacked)) {
                    if (!DB::ImageDB::instance()->getStackFor(a).isEmpty()) {
                        const auto stackedImages = DB::ImageDB::instance()->getStackFor(a);
                        for (const DB::FileName &b : stackedImages)
                            toBeShown.append(b);
                    } else
                        toBeShown.append(a);
                }
                DB::ImageDB::instance()->stack(stack);
            }
            showIfStacked.clear();
        }
    }
}

/**
 * This function searches for images that are shot within specified time frame
 */
void AutoStackImages::continuousShooting(DB::FileNameList &toBeShown)
{
    DB::ImageInfoPtr prev;
    for (const DB::FileName &fileName : std::as_const(m_list)) {
        const auto info = DB::ImageDB::instance()->info(fileName);
        // Skipping images that do not have exact time stamp
        if (info->date().start() != info->date().end())
            continue;
        if (prev && (prev->date().start().secsTo(info->date().start()) < m_continuousThreshold->value())) {
            DB::FileNameList stack;

            if (!DB::ImageDB::instance()->getStackFor(prev->fileName()).isEmpty()) {
                if (m_autostackUnstack->isChecked())
                    DB::ImageDB::instance()->unstack(DB::FileNameList() << prev->fileName());
                else if (m_autostackSkip->isChecked())
                    continue;
            }

            if (!DB::ImageDB::instance()->getStackFor(fileName).isEmpty()) {
                if (m_autostackUnstack->isChecked())
                    DB::ImageDB::instance()->unstack(DB::FileNameList() << fileName);
                else if (m_autostackSkip->isChecked())
                    continue;
            }

            stack.append(prev->fileName());
            stack.append(info->fileName());
            if (!toBeShown.isEmpty()) {
                if (toBeShown.at(toBeShown.size() - 1) != prev->fileName())
                    toBeShown.append(prev->fileName());
            } else {
                // if this is first insert, we have to include also the stacked images from previuous image
                if (!DB::ImageDB::instance()->getStackFor(info->fileName()).isEmpty()) {
                    const auto stackedImages = DB::ImageDB::instance()->getStackFor(prev->fileName());
                    for (const DB::FileName &a : stackedImages)
                        toBeShown.append(a);
                } else
                    toBeShown.append(prev->fileName());
            }
            // Inserting stacked images from the current image
            if (!DB::ImageDB::instance()->getStackFor(info->fileName()).isEmpty()) {
                const auto stackedImages = DB::ImageDB::instance()->getStackFor(fileName);
                for (const DB::FileName &a : stackedImages)
                    toBeShown.append(a);
            } else
                toBeShown.append(info->fileName());
            DB::ImageDB::instance()->stack(stack);
        }

        prev = info;
    }
}

void AutoStackImages::accept()
{
    QDialog::accept();
    Utilities::ShowBusyCursor dummy;
    DB::FileNameList toBeShown;

    if (m_matchingMD5->isChecked())
        matchingMD5(toBeShown);
    if (m_matchingFile->isChecked())
        matchingFile(toBeShown);
    if (m_continuousShooting->isChecked())
        continuousShooting(toBeShown);

    MainWindow::Window::theMainWindow()->showThumbNails(toBeShown);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_AutoStackImages.cpp"
