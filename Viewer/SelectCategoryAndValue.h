// SPDX-FileCopyrightText: 2023 Jesper K. Pedersen <jesper.pedersen@kdab.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <DB/CategoryPtr.h>
#include <QDialog>
#include <memory>

namespace Ui
{
class SelectCategoryAndValue;
}

class SelectCategoryAndValue : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCategoryAndValue(const QString &title, const QString &message, QWidget *parent = nullptr);
    ~SelectCategoryAndValue();
    QString category() const;
    QString value() const;
    int exec() override;

private:
    void addNew();

    const std::unique_ptr<Ui::SelectCategoryAndValue> ui;
    QString m_category;
    QString m_item;
};
