// SPDX-FileCopyrightText: 2012 - 2025 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "DuplicateMerger.h"

#include "MergeToolTip.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/MD5.h>
#include <ImageManager/AsyncLoader.h>
#include <MainWindow/DuplicateMerger/DuplicatesModel.h>
#include <Utilities/DeleteFiles.h>
#include <kpabase/FileName.h>
#include <kpabase/FileNameList.h>
#include <kpabase/Logging.h>

#include <KLocalizedString>
#include <QDebug>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

#include <utility>

/**
 * This class implements custom filtering for the duplicates model.
 */
class DuplicateSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit DuplicateSortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override
    {
        // Matches any filename in a row.  Column zero is the thumbnail.
        for (int col = 1; col < sourceModel()->columnCount(); col++) {
            QModelIndex index = sourceModel()->index(row, col, parent);

            if (sourceModel()->data(index).toString().contains(filterRegularExpression())) {
                return true;
            }
        }

        return false;
    }
};

namespace MainWindow
{
DuplicateMerger::DuplicateMerger(const DB::DuplicatesType& duplicates, QWidget *parent)
    : QDialog(parent)
    , m_model(new DuplicatesModel(duplicates, this))
    , m_filterProxy(new DuplicateSortFilterProxyModel(this))
{
    setModal(true);

    setAttribute(Qt::WA_DeleteOnClose);
    resize(800, 600);

    QWidget *top = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(top);
    setLayout(topLayout);
    topLayout->addWidget(top);

    QString txt = i18n("<p>Below is a list of all files that are duplicated in your database.<br/>"
                       "Select the files you want to keep.<br/>"
                       "Annotations from the deleted files will be merged to the kept image.</p>");
    QLabel *label = new QLabel(txt);
    QFont fnt = font();
    fnt.setPixelSize(18);
    label->setFont(fnt);
    topLayout->addWidget(label);

    m_trash = new QRadioButton(i18n("Move to &trash"));
    m_deleteFromDisk = new QRadioButton(i18n("&Delete from disk"));
    m_blockFromDB = new QRadioButton(i18n("&Block from database"));
    m_trash->setChecked(true);

    topLayout->addSpacing(10);
    topLayout->addWidget(m_trash);
    topLayout->addWidget(m_deleteFromDisk);
    topLayout->addWidget(m_blockFromDB);
    topLayout->addSpacing(10);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    topLayout->addLayout(horizontalLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    horizontalLayout->addLayout(vLayout);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(true);
    m_lineEdit->setPlaceholderText(i18nc("@label:textbox", "Filter ..."));
    m_lineEdit->setToolTip(i18n("Filter duplicates by filename"));

    vLayout->addWidget(m_lineEdit);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &DuplicateMerger::textChanged);

    m_lineEdit->installEventFilter(this);
    setFocusProxy(m_lineEdit);

    m_duplicatesView = new QTableView();
    m_filterProxy->setSourceModel(m_model);
    m_duplicatesView->setModel(m_filterProxy);

    m_duplicatesView->horizontalHeader()->setStretchLastSection(true);
    m_duplicatesView->verticalHeader()->hide();
    m_duplicatesView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_duplicatesView->verticalHeader()->setDefaultSectionSize(m_model->thumbnailSize().height());
    m_duplicatesView->resizeRowsToContents();
    m_duplicatesView->resizeColumnsToContents();
    connect(m_duplicatesView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DuplicateMerger::duplicatesTableSelectionChanged);
    vLayout->addWidget(m_duplicatesView);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    horizontalLayout->addLayout(buttonLayout);

    m_addButton = new QPushButton(i18n(">"), this);
    m_addButton->setEnabled(false);
    m_addButton->setToolTip(i18n("Add files to keep"));
    m_addButton->resize(m_addButton->minimumSize());
    connect(m_addButton, &QPushButton::clicked, this, &DuplicateMerger::addToKeepFiles);
    buttonLayout->addWidget(m_addButton);

    m_removeButton = new QPushButton(i18n("<"), this);
    m_removeButton->setEnabled(false);
    m_removeButton->setToolTip(i18n("Remove files to keep"));
    connect(m_removeButton, &QPushButton::clicked, this, &DuplicateMerger::removeFromKeepFiles);
    m_removeButton->resize(m_removeButton->minimumSize());
    buttonLayout->addWidget(m_removeButton);

    QVBoxLayout *listLayout = new QVBoxLayout;
    horizontalLayout->addLayout(listLayout);

    listLayout->addWidget(new QLabel(i18n("Files to keep:")));

    m_keepersList = new QListWidget();
    listLayout->addWidget(m_keepersList);
    m_keepersList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_keepersList, &QListWidget::itemSelectionChanged, this, &DuplicateMerger::keepersListSelectionChanged);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();

    m_selectNoneButton = buttonBox->addButton(i18n("Select &None"), QDialogButtonBox::NoRole);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_okButton->setEnabled(false);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(m_selectNoneButton, &QPushButton::clicked, this, &DuplicateMerger::selectNone);
    connect(m_okButton, &QPushButton::clicked, this, &DuplicateMerger::go);
    connect(m_cancelButton, &QPushButton::clicked, this, &DuplicateMerger::reject);

    topLayout->addWidget(buttonBox);
}

MainWindow::DuplicateMerger::~DuplicateMerger()
{
    MergeToolTip::destroy();
}

void DuplicateMerger::addToKeepFiles()
{
    m_addButton->setEnabled(false);
    for (const auto index : m_duplicatesView->selectionModel()->selectedIndexes()) {
        const QString fileName = m_model->data(m_model->index(index.row(), index.column()), Qt::DisplayRole).value<QString>();
        m_duplicatesView->setRowHidden(index.row(), true);
        m_indexes[fileName] = index.row();

        m_keepersList->addItem(new QListWidgetItem(fileName));
        qCDebug(ImageManagerLog) << __func__ << "added" << fileName;
    }

    m_keepersList->sortItems();
    m_okButton->setEnabled(m_keepersList->count() > 0);
}

void DuplicateMerger::keepersListSelectionChanged()
{
    m_removeButton->setEnabled(!m_keepersList->selectedItems().isEmpty());
}

void DuplicateMerger::removeFromKeepFiles()
{
    qCDebug(ImageManagerLog) << __func__ << "clicked";
    for (const auto item : m_keepersList->selectedItems()) {
        const auto fileName = item->data(Qt::DisplayRole).value<QString>();
        m_keepersList->takeItem(m_keepersList->row(item));
        m_duplicatesView->setRowHidden(m_indexes.take(fileName), false);
        qCDebug(ImageManagerLog) << __func__ << "removed" << fileName;
        delete item;
    }

    m_okButton->setEnabled(m_keepersList->count() > 0);
}

void DuplicateMerger::selectNone()
{
    m_duplicatesView->selectionModel()->clearSelection();
}

void DuplicateMerger::go()
{
    Utilities::DeleteMethod method = Utilities::RemoveFromDatabase;

    if (m_trash->isChecked()) {
        method = Utilities::MoveToTrash;
    } else if (m_deleteFromDisk->isChecked()) {
        method = Utilities::DeleteFromDisk;
    }

    // For each row in m_keepersList, use the hash to get the (hidden) row in
    // the duplicates table.  The keepers table provides the filename to keep
    // and the files to remove are the other filenames from m_model for that row.
    DB::FileNameList deleteList, dupList;

    for (auto row = 0; row < m_keepersList->count(); row++) {
        QListWidgetItem* item = m_keepersList->item(row);
        const auto keeperFileName = item->data(Qt::DisplayRole).value<QString>();
        const auto modelRow = m_indexes.take(keeperFileName);

        qCDebug(ImageManagerLog) << "Keeping" << keeperFileName;

        // Column zero is the pixmap.
        for (int column = 1; column < m_model->columnCount(); column++) {
            const auto i = m_model->index(modelRow, column);
            const auto data = m_model->data(i, Qt::DisplayRole);

            // Rows in the table can be different lengths.
            if (data.isValid()) {
                const auto fileName = data.value<QString>();

                if (fileName != keeperFileName) {
                    const auto relPath = DB::FileName::fromRelativePath(data.value<QString>());

                    DB::ImageDB::instance()->copyData(relPath, DB::FileName::fromRelativePath(keeperFileName));
                    qCDebug(ImageManagerLog) << "Deleting" << fileName;
                    deleteList.append(relPath);
                }
            }
        }
    }

    Utilities::DeleteFiles::deleteFiles(deleteList, method);
    // remove duplicate DB-entries without removing or blocking the file:
    DB::ImageDB::instance()->deleteList(dupList); // TODO

    accept();
}

void DuplicateMerger::textChanged(const QString &str)
{
    m_filterProxy->setFilterRegularExpression(str);
}

void DuplicateMerger::duplicatesTableSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    // Counts the number of selected cells in each row.  The key is the row
    // index.
    QMap<unsigned, unsigned> counts;

    for (auto i : m_duplicatesView->selectionModel()->selectedIndexes()) {
        counts[i.row()]++;
    }

    unsigned maxCount = 0;

    for (auto count : counts) {
        if (count > maxCount) {
            maxCount = count;
        }
    }

    // Enable only if at least one row has one selected cell and no row has
    // more than one selected cell.
    m_addButton->setEnabled(maxCount == 1);
}
} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DuplicateMerger.cpp"
