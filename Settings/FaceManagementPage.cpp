/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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

#include "FaceManagementPage.h"

// Qt includes
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>
#include <QTreeWidget>
#include <QAbstractItemView>
#include <QPushButton>
#include <QDebug>

// KDE includes
#include <KLocale>
#include <KMessageBox>
#include <KPageWidgetItem>

// Local includes
#include "SettingsData.h"
#include "FaceManagement/Recognizer.h"

Settings::FaceManagementPage::FaceManagementPage(QWidget* parent) : QWidget(parent)
{
    // The main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    ////////////////////////////////
    // The detection settings box //
    ////////////////////////////////

    QGroupBox* detectionBox = new QGroupBox(i18n("Face detection"));
    mainLayout->addWidget(detectionBox);

    // The detection settings layout
    QGridLayout* detectionLayout = new QGridLayout(detectionBox);

    // Speed

    QLabel* speedLabel = new QLabel(i18n("Accuracy"));
    speedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    detectionLayout->addWidget(speedLabel, 0, 1);

    QLabel* fastLabel = new QLabel(i18n("fast"));
    fastLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    detectionLayout->addWidget(fastLabel, 1, 0);

    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setMaximum(100);
    m_speedSlider->setTickInterval(10);
    m_speedSlider->setTickPosition(QSlider::TicksBothSides);
    detectionLayout->addWidget(m_speedSlider, 1, 1);

    QLabel* accurateLabel = new QLabel(i18n("accurate"));
    detectionLayout->addWidget(accurateLabel, 1, 2);

    // Sensitivity

    QLabel* sensitivityLabel = new QLabel(i18n("Sensitivity"));
    sensitivityLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    detectionLayout->addWidget(sensitivityLabel, 2, 1);

    QLabel* falsePositivesLabel = new QLabel(i18n("more false positives"));
    falsePositivesLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    detectionLayout->addWidget(falsePositivesLabel, 3, 0);

    m_sensitivitySlider = new QSlider(Qt::Horizontal);
    m_sensitivitySlider->setMaximum(100);
    m_sensitivitySlider->setTickInterval(10);
    m_sensitivitySlider->setTickPosition(QSlider::TicksBothSides);
    detectionLayout->addWidget(m_sensitivitySlider, 3, 1);

    QLabel* missedFacesLabel = new QLabel(i18n("more missed faces"));
    detectionLayout->addWidget(missedFacesLabel, 3, 2);

    // Make the detection settings box as small as possible
    QSizePolicy detectionPolicy;
    detectionPolicy.setHorizontalPolicy(QSizePolicy::Minimum);
    detectionBox->setSizePolicy(detectionPolicy);

    //////////////////////////////////
    // The recognition database box //
    //////////////////////////////////

    QGroupBox* recognitionBox = new QGroupBox(i18n("Face recognition"));
    mainLayout->addWidget(recognitionBox);

    // The detection settings layout
    QGridLayout* recognitionLayout = new QGridLayout(recognitionBox);

    // List of all database entries
    m_databaseEntries = new QTreeWidget;
    m_databaseEntries->setColumnCount(1);
    m_databaseEntries->setHeaderLabels(QStringList(i18n("Database entry")));
    m_databaseEntries->setSortingEnabled(true);
    m_databaseEntries->setSelectionMode(QAbstractItemView::ExtendedSelection);
    clearDatabaseEntries();
    recognitionLayout->addWidget(m_databaseEntries, 0, 0, 3, 1);

    // Take as much space as possible for the database entries list
    QSizePolicy databaseEntriesPolicy;
    databaseEntriesPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    databaseEntriesPolicy.setVerticalPolicy(QSizePolicy::Expanding);
    m_databaseEntries->setSizePolicy(databaseEntriesPolicy);

    connect(m_databaseEntries, SIGNAL(itemSelectionChanged()), this, SLOT(checkSelection()));

    // The "Delete selected" button
    m_deleteSelectedButton = new QPushButton(i18n("Delete selected"));
    m_deleteSelectedButton->setEnabled(false);
    recognitionLayout->addWidget(m_deleteSelectedButton, 1, 1);
    connect(m_deleteSelectedButton, SIGNAL(clicked()), this, SLOT(slotDeleteSelected()));

    // The "Erase database" button
    QPushButton* eraseButton = new QPushButton(i18n("Erase database"));
    recognitionLayout->addWidget(eraseButton, 2, 1);
    connect(eraseButton, SIGNAL(clicked()), this, SLOT(slotEraseDatabase()));

    QLabel* cautionLabel = new QLabel(i18n(
        "<b>Caution:</b> Changes to category names or category deletions are not "
        "adopted by the recognition database until the changes are saved!"), this);
    mainLayout->addWidget(cautionLabel);
    cautionLabel->setSizePolicy(detectionPolicy);
}

Settings::FaceManagementPage::~FaceManagementPage()
{
}

void Settings::FaceManagementPage::clearDatabaseEntries()
{
    if (m_databaseEntries->isEnabled()) {
        m_databaseEntries->clear();
        m_databaseEntries->setEnabled(false);
    }
}

void Settings::FaceManagementPage::checkSelection()
{
    if (m_databaseEntries->selectedItems().size() > 0) {
        m_deleteSelectedButton->setEnabled(true);
    } else {
        m_deleteSelectedButton->setEnabled(false);
    }
}

void Settings::FaceManagementPage::loadSettings(Settings::SettingsData* opt)
{
    m_speedSlider->setSliderPosition(opt->faceDetectionAccuracy());
    m_sensitivitySlider->setSliderPosition(opt->faceDetectionSensitivity());
    loadDatabase();
}

void Settings::FaceManagementPage::saveSettings(Settings::SettingsData* opt)
{
    opt->setFaceDetectionAccuracy(m_speedSlider->value());
    opt->setFaceDetectionSensitivity(m_sensitivitySlider->value());
}

void Settings::FaceManagementPage::loadDatabase()
{
    m_databaseEntries->clear();
    m_databaseEntries->setSortingEnabled(false);
    m_recognizer = FaceManagement::Recognizer::instance();

    QMap<QString, QStringList> parsedIdentities = m_recognizer->allParsedIdentities();
    QMapIterator<QString, QStringList> tagList(parsedIdentities);
    while (tagList.hasNext()) {
        tagList.next();

        QTreeWidgetItem* category = new QTreeWidgetItem;
        category->setText(0, DB::Category::localizedCategoryName(tagList.key()));
        category->setFlags(Qt::ItemIsEnabled);

        for (int i = 0; i < tagList.value().size(); ++i) {
            QTreeWidgetItem* tag = new QTreeWidgetItem;
            tag->setText(0, tagList.value().at(i));
            tag->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            category->addChild(tag);
        }

        m_databaseEntries->addTopLevelItem(category);
        category->setExpanded(true);
    }

    m_databaseEntries->setEnabled(true);
    m_databaseEntries->setSortingEnabled(true);
    m_databaseEntries->sortItems(0, Qt::AscendingOrder);
}

void Settings::FaceManagementPage::slotDeleteSelected()
{
    int answer = KMessageBox::questionYesNo(this, i18n("Do you really want to delete the selected tags?"));
    if (answer == KMessageBox::No) {
        return;
    }

    QList<QPair<QString, QString>> tagsToDelete;
    QList<QTreeWidgetItem*> selectedItems = m_databaseEntries->selectedItems();
    for (int i = 0; i < selectedItems.size(); ++i) {
        tagsToDelete << QPair<QString, QString>(
            selectedItems.at(i)->parent()->text(0),
            selectedItems.at(i)->text(0)
        );
    }

    m_recognizer->deleteTags(tagsToDelete);

    if (m_recognizer->allParsedIdentities().size() > 0) {
        loadDatabase();
    } else {
        clearDatabaseEntries();
    }
}

void Settings::FaceManagementPage::slotEraseDatabase()
{
    int answer = KMessageBox::questionYesNo(this, i18n("Do you really want to erase the database?"));
    if (answer == KMessageBox::No) {
        return;
    }

    m_recognizer = FaceManagement::Recognizer::instance();
    m_recognizer->eraseDatabase();

    clearDatabaseEntries();
}

void Settings::FaceManagementPage::slotPageChange(KPageWidgetItem* page)
{
    if (page->widget() == this) {
        loadDatabase();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
