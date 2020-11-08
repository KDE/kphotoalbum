/* SPDX-FileCopyrightText: 2016 Tobias Leupold <tobias.leupold@web.de>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef AREATAGSELECTDIALOG_H
#define AREATAGSELECTDIALOG_H

// Qt includes
#include <QDialog>
#include <QSet>
#include <QString>

// Qt classes
class QLabel;
class QPaintEvent;

namespace AnnotationDialog
{

class Dialog;
class ListSelect;
class ResizableFrame;

/**
 * @brief The AreaTagSelectDialog is the dialog that pops up when
 * a new area is drawn on the ImagePreview.
 */
class AreaTagSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AreaTagSelectDialog(ResizableFrame *area,
                                 ListSelect *ls,
                                 QPixmap areaImage,
                                 Dialog *dialog);
    void moveToArea(QPoint areaTopLeft);

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void slotSetTag(const QString &tag);
    void slotValidateTag(const QString &tag);
    void slotFinished();

private:
    QLabel *m_areaImageLabel;
    ResizableFrame *m_area;
    ListSelect *m_listSelect;
    Dialog *m_dialog;
    QLabel *m_messageLabel;
    const QSet<QString> m_usedTags;
};

}

#endif // AREATAGSELECTDIALOG_H

// vi:expandtab:tabstop=4 shiftwidth=4:
