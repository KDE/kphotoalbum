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
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <utility>

// Width and height of the thumbnail preview pixmaps.
const auto PREVIEW_SIZE = QSize(100, 100);

namespace MainWindow
{

DuplicateMerger::DuplicateMerger(QWidget *parent)
    : QDialog(parent)
    , m_model(new DuplicatesModel(this))
{
    setModal(true);

    setAttribute(Qt::WA_DeleteOnClose);
    resize(800, 600);

    m_filterProxy = new QSortFilterProxyModel(this);
    m_filterProxy->setSourceModel(m_model);

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

    // QHBoxLayout *horizontalLayout = new QHBoxLayout;
    // topLayout->addLayout(horizontalLayout);

    m_trash = new QRadioButton(i18n("Move to &trash"));
    m_deleteFromDisk = new QRadioButton(i18n("&Delete from disk"));
    m_blockFromDB = new QRadioButton(i18n("&Block from database"));
    m_trash->setChecked(true);

    topLayout->addSpacing(10);
    topLayout->addWidget(m_trash);
    topLayout->addWidget(m_deleteFromDisk);
    topLayout->addWidget(m_blockFromDB);
    topLayout->addSpacing(10);

    m_previewWidget = new QLabel;
    topLayout->addWidget(m_previewWidget);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(true);
    m_lineEdit->setPlaceholderText(i18nc("@label:textbox", "Filter ..."));
    m_lineEdit->setToolTip(i18n("Filter by filename"));

    topLayout->addWidget(m_lineEdit);
    connect(m_lineEdit, &QLineEdit::textChanged, this, &DuplicateMerger::textChanged);
    // connect(m_lineEdit, &QLineEdit::returnPressed, this, &DuplicateMerger::returnPressed);

    m_lineEdit->installEventFilter(this);
    setFocusProxy(m_lineEdit);

    m_selectionCount = new QLabel;

    QDialogButtonBox *buttonBox = new QDialogButtonBox();

    m_selectNoneButton = buttonBox->addButton(i18n("Select &None"), QDialogButtonBox::NoRole);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);

    connect(m_selectNoneButton, &QPushButton::clicked, this, &DuplicateMerger::selectNone);
    connect(m_okButton, &QPushButton::clicked, this, &DuplicateMerger::go);
    connect(m_cancelButton, &QPushButton::clicked, this, &DuplicateMerger::reject);

    // TODO: can this be moved out of the UX code?
    findDuplicates();

    m_tableView = new QTableView();
    m_tableView->setModel(m_model);
    m_tableView->verticalHeader()->hide();
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setModel(m_filterProxy);
    m_tableView->resizeRowsToContents();
    m_tableView->resizeColumnsToContents();
    m_model->setParent(m_tableView);
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DuplicateMerger::selectionChanged);
    // connect(m_tableView->selectionModel(), &QItemSelectionModel::dataChanged, this, &DuplicateMerger::selectionChanged);
    topLayout->addWidget(m_tableView);

    topLayout->addWidget(m_selectionCount);

    topLayout->addWidget(buttonBox);
}

MainWindow::DuplicateMerger::~DuplicateMerger()
{
    MergeToolTip::destroy();
}

void DuplicateMerger::selectNone()
{
    m_tableView->selectionModel()->clearSelection();
}

void DuplicateMerger::go()
{
    Utilities::DeleteMethod method = Utilities::RemoveFromDatabase;

    if (m_trash->isChecked()) {
        method = Utilities::MoveToTrash;
    } else if (m_deleteFromDisk->isChecked()) {
        method = Utilities::DeleteFromDisk;
    }

    // For each row, if a filename is selected then delete the unselected
    // filenames in that row.
    // TODO: need the delete/block logic from DuplicateMatch here!
    for (auto index : m_tableView->selectionModel()->selectedIndexes()) {
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
}

void DuplicateMerger::findDuplicates()
{
    Utilities::ShowBusyCursor dummy;

    const auto images = DB::ImageDB::instance()->files();
    for (const DB::FileName &fileName : images) {
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const DB::MD5 md5 = info->MD5Sum();
        m_matches[md5].append(fileName);
    }

    // Sort any duplicates in order of increasing birth time to make the
    // positioning in the table consistent (ie. the oldest duplicate in each
    // row is in the first column and the newest duplicate is in the last
    // column).
    for (QMap<DB::MD5, DB::FileNameList>::iterator it = m_matches.begin();
         it != m_matches.end(); ++it) {
        if (it.value().count() > 1) {
            std::sort(it.value().begin(), it.value().end(),
                      [](DB::FileName a, DB::FileName b) {
                          const QFileInfo aInfo(a.absolute());
                          const QFileInfo bInfo(b.absolute());

                          return aInfo.birthTime() < bInfo.birthTime();
                      });
        }
    }

    // This is used to sort the rows (selectors) in the dialog by relative
    // pathname of the oldest image in each set of duplicates.
    QMap<QString, DB::MD5> displayOrderMap;

    for (QMap<DB::MD5, DB::FileNameList>::const_iterator it = m_matches.constBegin();
         it != m_matches.constEnd(); ++it) {
        if (it.value().count() > 1) {
            displayOrderMap.insert(it.value().first().relative(), it.key());
        }
    }

    if (displayOrderMap.empty()) {
        tellThatNoDuplicatesWereFound();
    } else {
        for (DB::MD5 md5 : displayOrderMap.values()) {
            m_model->addDuplicates(m_matches[md5]);
        }
    }

    updateSelectionCount();
}

void DuplicateMerger::tellThatNoDuplicatesWereFound()
{
    QLabel *label = new QLabel(i18n("No duplicates found"));
    QFont fnt = font();
    fnt.setPixelSize(30);
    label->setFont(fnt);

    m_trash->setEnabled(false);
    m_deleteFromDisk->setEnabled(false);
    m_blockFromDB->setEnabled(false);

    m_selectNoneButton->setEnabled(false);
    m_okButton->setEnabled(false);
}

void DuplicateMerger::textChanged(const QString &str)
{
    m_filterProxy->setFilterFixedString(str);
}

void DuplicateMerger::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // TODO: check that no indexes in selected are blank...
    // TODO: don't allow selections in the pixmap column
    // TODO: only one selection per row
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    updateSelectionCount(m_tableView->selectionModel()->selectedIndexes().size());
}

DuplicatesModel::DuplicatesModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_maxDuplicates(0)
{
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
    } else if (section == 1) {
        return i18nc("@title:column Original file", "Original");
    } else {
        return i18nc("@title:column Duplicate file", "Duplicate");
    }
}
} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DuplicateMerger.cpp"
