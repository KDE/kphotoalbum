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
#include <Utilities/ShowBusyCursor.h>
#include <kpabase/FileName.h>
#include <kpabase/FileNameList.h>
#include <kpabase/Logging.h>

#include <KLocalizedString>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileInfo>
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

// Width and height of the thumbnail preview pixmaps.
const auto PREVIEW_SIZE = QSize(100, 100);

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
        // Matches any filename in a row.
        for (int col = 1; col < sourceModel()->columnCount(); col++) {
            QModelIndex index = sourceModel()->index(row, col, parent);

            if (sourceModel()->data(index).toString().contains(filterRegularExpression())) {
                return true;
            }
        }

        return false;
    }
};

/**
 * This class exists to customize the selection behavior for the duplicates table.
 */
class DuplicatesTableView : public QTableView
{
protected:
    virtual bool edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event) override
    {
        if (index.column() == 0) {
            // Don't allow selection in column zero (the pixmap column).
            return true;
        }

        return QAbstractItemView::edit(index, trigger, event);
    }

protected:
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event = nullptr) const override
    {
        if (event == nullptr && index.column() == 0) {
            // Don't allow selection of column zero (the pixmap column) by
            // clicking on the column header.
            return QItemSelectionModel::NoUpdate;
        }

        return QAbstractItemView::selectionCommand(index, event);
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

    QString txt = i18n("<p>Below is a list of all images that are duplicated in your database.<br/>"
                       "Select which you want merged, and which of the duplicates should be kept.<br/>"
                       "The tag and description from the deleted images will be merged to the kept image.</p>");
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

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(true);
    m_lineEdit->setPlaceholderText(i18nc("@label:textbox", "Filter ..."));
    m_lineEdit->setToolTip(i18n("Filter by filename"));

    topLayout->addWidget(m_lineEdit);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &DuplicateMerger::textChanged);
    // connect(m_lineEdit, &QLineEdit::textChanged, m_filterProxy, &DuplicateSortFilterProxyModel::setFilterRegularExpression);

    m_lineEdit->installEventFilter(this);
    setFocusProxy(m_lineEdit);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    topLayout->addLayout(horizontalLayout);

    m_duplicatesView = new DuplicatesTableView();
    m_filterProxy->setSourceModel(m_model);
    m_duplicatesView->setModel(m_filterProxy);

    m_duplicatesView->horizontalHeader()->setStretchLastSection(true);
    m_duplicatesView->verticalHeader()->hide();
    m_duplicatesView->resizeRowsToContents();
    m_duplicatesView->resizeColumnsToContents();
    // m_duplicatesView->setSelectionMode(QAbstractItemView::NoSelection);
    connect(m_duplicatesView, &QTableView::clicked, this, &MainWindow::DuplicateMerger::duplicateClicked);
    connect(m_duplicatesView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DuplicateMerger::selectionChanged);
    // connect(m_duplicatesView->selectionModel(), &QItemSelectionModel::dataChanged, this, &DuplicateMerger::selectionChanged);
    horizontalLayout->addWidget(m_duplicatesView);

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

    connect(m_keepersList->model(), &QAbstractTableModel::rowsInserted, this, &DuplicateMerger::enableAddToKeepFiles);

    m_selectionCount = new QLabel;
    topLayout->addWidget(m_selectionCount);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();

    m_selectNoneButton = buttonBox->addButton(i18n("Select &None"), QDialogButtonBox::NoRole);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
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

void DuplicateMerger::duplicateClicked(const QModelIndex &index)
{
    qCDebug(ImageManagerLog) << __func__ << "clicked" << index;
    m_addButton->setEnabled(true);
}

void DuplicateMerger::addToKeepFiles()
{
    m_addButton->setEnabled(false);
    for (const auto index : m_duplicatesView->selectionModel()->selectedIndexes()) {
        const QString fileName = m_model->data(m_model->index(index.row(), index.column()), Qt::DisplayRole).value<QString>();
        m_duplicatesView->setRowHidden(index.row(), true);
        m_indexes[fileName] = index.row();

        m_keepersList->addItem(new QListWidgetItem(fileName));
        qCDebug(ImageManagerLog) << __func__ << "removed" << fileName;
    }

    m_keepersList->sortItems();
}

void DuplicateMerger::enableAddToKeepFiles(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    m_removeButton->setEnabled(true);
}

void DuplicateMerger::removeFromKeepFiles()
{
    qCDebug(ImageManagerLog) << __func__ << "clicked";
    for (const auto item : m_keepersList->selectedItems()) {
        const auto fileName = item->data(Qt::DisplayRole).value<QString>();
        m_keepersList->takeItem(m_keepersList->row(item));
        m_duplicatesView->setRowHidden(m_indexes.take(fileName), false);
        qCDebug(ImageManagerLog) << __func__ << "removed" << fileName;
    }

    m_removeButton->setEnabled(m_keepersList->model()->rowCount() > 0);
}

void DuplicateMerger::selectNone()
{
    m_duplicatesView->selectionModel()->clearSelection();
}

void DuplicateMerger::go()
{
    // Utilities::DeleteMethod method = Utilities::RemoveFromDatabase;

    // if (m_trash->isChecked()) {
    //     method = Utilities::MoveToTrash;
    // } else if (m_deleteFromDisk->isChecked()) {
    //     method = Utilities::DeleteFromDisk;
    // }

    // For each row, if a filename is selected then delete the unselected
    // filenames in that row.
    // TODO: need the delete/block logic from DuplicateMatch here!
    for (auto index : m_duplicatesView->selectionModel()->selectedIndexes()) {
        // Column zero is the pixmap.
        for (int column = 1; column < m_model->columnCount(); column++) {
            if (column != index.column()) {
                const auto i = m_model->index(index.row(), column);
                const auto fileName = m_model->data(i, Qt::DisplayRole);
                // Rows in the table can be different lengths.
                if (fileName.isValid()) {
                    qCDebug(ImageManagerLog) << "Delete" << m_model->data(i, Qt::DisplayRole);
                }
            }
        }
    }
    // TODO: can this be moved out of the UX code?
    // for (DuplicateMatch *selector : std::as_const(m_selectors)) {
    //     selector->execute(method);
    // }

    accept();
}

void DuplicateMerger::updateSelectionCount(qsizetype selectionCount)
{
    m_selectionCount->setText(i18n("%1 of %2 selected", selectionCount, m_model->rowCount()));
    m_okButton->setEnabled(selectionCount > 0);
    m_addButton->setEnabled(selectionCount > 0);
}

void DuplicateMerger::textChanged(const QString &str)
{
    m_filterProxy->setFilterRegularExpression(str);
}

void DuplicateMerger::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // TODO: check that no indexes in selected are blank...
    // TODO: don't allow selections in the pixmap column
    // TODO: only one selection per row
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    updateSelectionCount(m_duplicatesView->selectionModel()->selectedIndexes().size());
}

DuplicatesModel::DuplicatesModel(const DB::DuplicatesType& duplicates, QObject* parent)
    : QAbstractTableModel(parent)
    , m_maxDuplicates(0)
{
    // This is used to sort the rows in the duplicates table by relative
    // pathname of the oldest image in each set of duplicates.
    QMap<QString, DB::MD5> displayOrderMap;

    for (QMap<DB::MD5, DB::FileNameList>::const_iterator it = duplicates.constBegin();
         it != duplicates.constEnd(); ++it) {
        if (it.value().count() > 1) {
            displayOrderMap.insert(it.value().first().relative(), it.key());
        }
    }

    for (DB::MD5 md5 : displayOrderMap.values()) {
        addDuplicates(duplicates[md5]);
    }
}

DuplicatesModel::~DuplicatesModel()
{
}

void DuplicatesModel::addDuplicates(const DB::FileNameList &files)
{
    m_files << files;
    if (files.size() > m_maxDuplicates) {
        m_maxDuplicates = files.size();
    }

    const auto &originalFileName = files.first();
    const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(originalFileName);
    const int angle = info->angle();
    ImageManager::ImageRequest *request = new ImageManager::ImageRequest(originalFileName, PREVIEW_SIZE, angle, this);
    ImageManager::AsyncLoader::instance()->load(request);
}

void DuplicatesModel::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    m_pixmaps[request->databaseFileName().relative()] = QPixmap::fromImage(image);
    qCDebug(ImageManagerLog) << "Loaded pixmap for" << request->databaseFileName().relative();
}

QVariant DuplicatesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    const int column = index.column();

    if (row < m_files.count() && column < columnCount()) {
        if (role == Qt::DisplayRole) {
            // Column zero is the pixmap.
            if (column > 0) {
                if (column <= m_files[row].size()) {
                    return m_files[row][column-1].relative();
                }
            }
        }
        else if (role == Qt::DecorationRole) {
            if (column == 0) {
                const auto &originalFileName = m_files[row].first().relative();
                qCDebug(ImageManagerLog) << "Get pixmap data for" << originalFileName;
                return m_pixmaps[originalFileName];
            }
        }
        else if (role == Qt::SizeHintRole) {
            if (column == 0) {
                const auto &originalFileName = m_files[row].first().relative();
                qCDebug(ImageManagerLog) << "Get pixmap size for" << originalFileName;
                return PREVIEW_SIZE;
            }
        }
    }

    return QVariant();
}

QVariant DuplicatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    if (section == 0) {
        return i18nc("@title:column Image preview", "Preview");
    } else {
        return i18nc("@title:column Duplicate file", "Duplicate %1", section);
    }
}
} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DuplicateMerger.cpp"
