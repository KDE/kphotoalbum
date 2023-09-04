// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "SelectCategoryAndValue.h"
#include "ui_SelectCategoryAndValue.h"
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>

#include <KLocalizedString>
#include <QCompleter>
#include <QIdentityProxyModel>
#include <QMenu>
#include <QPushButton>
#include <QStandardItem>

#include <algorithm>

Q_DECLARE_METATYPE(DB::CategoryPtr)

namespace
{
/**
    This proxy model adapts its filtering according to the current search.
    This is to allow the search to match "Person / Jesper" by simply searching for "Per Je"
*/
class AdaptiveFilterProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    using QIdentityProxyModel::QIdentityProxyModel;

    static constexpr int SearchRole = 2000;
    void setText(const QString &text)
    {
        if (text == m_text)
            return;
        m_text = text;
        m_subStrings = text.split(QLatin1String(" "));
        // This signal must be emitted to invalidate the completor, and force it to filter its rows
        // again.
        Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), { SearchRole });
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role != SearchRole)
            return QIdentityProxyModel::data(index, role);

        auto matches = [itemText = data(index, Qt::DisplayRole).toString()](const QString &str) {
            return itemText.contains(str, Qt::CaseInsensitive);
        };

        if (!m_text.isEmpty() && std::all_of(m_subStrings.cbegin(), m_subStrings.cend(), matches)) {
            // If the item matches then simply return the search string which will make the
            // completer include this match.
            return m_text;
        } else {
            // Otherwise return an empty string which will make the completer discard this item.
            return QString();
        }
    }

private:
    QStringList m_subStrings;
    QString m_text;
};

enum Role { Category = Qt::UserRole,
            Item };

} // namespace

SelectCategoryAndValue::SelectCategoryAndValue(const QString &title, const QString &message, const Viewer::AnnotationHandler::Assignments &assignments, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectCategoryAndValue)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->label->setText(message);
    setupExistingAssignments(assignments);

    auto model = new QStandardItemModel(this);
    int row = 0;

    const auto sortOrder = DB::ImageDB::instance()->categoryCollection()->globalSortOrder()->completeSortOrder();
    for (const auto &item : sortOrder) {
        auto modelItem = new QStandardItem(QLatin1String("%1 / %2").arg(item.category, item.item));
        modelItem->setData(item.category, Role::Category);
        modelItem->setData(item.item, Role::Item);
        model->insertRow(row++, modelItem);
    }

    auto searchFilter = new AdaptiveFilterProxy(this);
    searchFilter->setSourceModel(model);

    auto completer = new QCompleter(this);
    ui->lineEdit->setCompleter(completer);
    completer->setModel(searchFilter);
    completer->setFilterMode(Qt::MatchStartsWith);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCompletionRole(AdaptiveFilterProxy::SearchRole);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    connect(ui->lineEdit, &QLineEdit::textChanged, searchFilter, &AdaptiveFilterProxy::setText);

    ui->lineEdit->setPlaceholderText(i18n("Type name of category and item"));

    connect(ui->lineEdit, &QLineEdit::returnPressed, this, [this, completer] {
        auto index = completer->currentIndex();
        if (index.isValid()) {
            m_category = index.data(Role::Category).toString();
            m_item = index.data(Role::Item).toString();
        }
        accept();
    });

    connect(
        completer, qOverload<const QModelIndex &>(&QCompleter::activated), this,
        [this](const QModelIndex &index) {
            m_category = index.data(Role::Category).toString();
            m_item = index.data(Role::Item).toString();
            accept();
        });

    connect(ui->lineEdit, &QLineEdit::textChanged, ui->buttonBox, [completer, this] {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(completer->currentIndex().isValid());
    });

    connect(ui->value, &QLineEdit::textChanged, ui->buttonBox, [this](const QString &txt) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!txt.isEmpty());
    });

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->addNewTag->setText(i18n("Add New"));
    connect(ui->addNewTag, &QPushButton::clicked, this, &SelectCategoryAndValue::addNew);
    ui->categoryLabel->hide();
    ui->category->hide();
    ui->titleLabel->hide();
    ui->value->hide();

    connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &SelectCategoryAndValue::helpRequest);

    const auto categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const auto &category : categories) {
        if (!category->isSpecialCategory())
            ui->category->addItem(category->name(), QVariant::fromValue(category));
    }

    ui->knownAssignments->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->knownAssignments, &QWidget::customContextMenuRequested, this, &SelectCategoryAndValue::knownAssignmentsContextMenu);
}

SelectCategoryAndValue::~SelectCategoryAndValue() = default;

int SelectCategoryAndValue::exec()
{
    auto result = QDialog::exec();
    if (result == QDialog::Rejected)
        return result;

    if (!ui->category->isHidden()) {
        m_category = ui->category->currentText();
        m_item = ui->value->text();
        ui->category->currentData().value<DB::CategoryPtr>()->addItem(m_item);
    }

    DB::ImageDB::instance()->categoryCollection()->globalSortOrder()->pushToFront(m_category, m_item);
    return result;
}

void SelectCategoryAndValue::addNew()
{
    ui->lineEdit->hide();
    ui->addNewTag->hide();
    ui->categoryLabel->show();
    ui->category->show();
    ui->titleLabel->show();
    ui->value->show();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->value->setText(ui->lineEdit->text());
    ui->category->setFocus();
}

void SelectCategoryAndValue::setupExistingAssignments(const Viewer::AnnotationHandler::Assignments &assignments)
{
    auto model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList { i18n("Key"), i18n("Tag") });
    int row = 0;
    auto addAssignment = [&](const QString &key, const Viewer::AnnotationHandler::Assignment &assignment) {
        auto *keyItem = new QStandardItem(key);
        keyItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        model->setItem(row, 0, keyItem);
        auto *assignmentItem = new QStandardItem(QLatin1String("%1 / %2").arg(assignment.category, assignment.value));
        assignmentItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        model->setItem(row, 1, assignmentItem);
        ++row;
    };

    for (auto it = assignments.cbegin(); it != assignments.cend(); ++it) {
        const Viewer::AnnotationHandler::Assignment assignment = it.value();
        addAssignment(it.key(), assignment);
    }

    ui->knownAssignments->setModel(model);
    ui->knownAssignments->verticalHeader()->hide();
    ui->knownAssignments->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->knownAssignments->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->knownAssignments->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void SelectCategoryAndValue::knownAssignmentsContextMenu(const QPoint &point)
{
    const auto &index = ui->knownAssignments->indexAt(point);
    if (index.isValid()) {
        QMenu contextMenu { this };
        QAction deleteItemAction { this };
        contextMenu.addAction(&deleteItemAction);
        deleteItemAction.setText(i18nc("@action:inmenu", "Clear assignment"));
        connect(&deleteItemAction, &QAction::triggered, ui->knownAssignments, [&] {
            // initiate removal from config:
            const auto assignmentKey = index.siblingAtColumn(0);
            if (assignmentKey.isValid()) {
                // clear assignment:
                const auto key = ui->knownAssignments->model()->data(assignmentKey).toString();
                Q_EMIT keyRemovalRequested(key);

                // update table view:
                ui->knownAssignments->model()->removeRow(index.row());
            }
        });
        contextMenu.exec(ui->knownAssignments->mapToGlobal(point));
    }
}

QString SelectCategoryAndValue::category() const
{
    return m_category;
}

QString SelectCategoryAndValue::value() const
{
    return m_item;
}

#include "SelectCategoryAndValue.moc"
