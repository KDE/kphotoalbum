/* Copyright 2012-2018 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DuplicateMatch.h"

#include <QCheckBox>
#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVariant>

#include <KLocalizedString>

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <ImageManager/AsyncLoader.h>
#include <Utilities/DeleteFiles.h>

#include "MergeToolTip.h"

namespace MainWindow
{

DuplicateMatch::DuplicateMatch(const DB::FileNameList &files)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    topLayout->addLayout(horizontalLayout);
    m_image = new QLabel;
    horizontalLayout->addWidget(m_image);

    QVBoxLayout *rightSideLayout = new QVBoxLayout;
    horizontalLayout->addSpacing(20);
    horizontalLayout->addLayout(rightSideLayout);
    horizontalLayout->addStretch(1);
    rightSideLayout->addStretch(1);

    m_merge = new QCheckBox(i18n("Merge these images"));
    rightSideLayout->addWidget(m_merge);
    m_merge->setChecked(false);
    connect(m_merge, SIGNAL(toggled(bool)), this, SIGNAL(selectionChanged()));

    QWidget *options = new QWidget;
    rightSideLayout->addWidget(options);
    QVBoxLayout *optionsLayout = new QVBoxLayout(options);
    connect(m_merge, SIGNAL(toggled(bool)), options, SLOT(setEnabled(bool)));

    QLabel *label = new QLabel(i18n("Select target:"));
    optionsLayout->addWidget(label);

    bool first = true;
    Q_FOREACH (const DB::FileName &fileName, files) {
        QHBoxLayout *lay = new QHBoxLayout;
        optionsLayout->addLayout(lay);
        QRadioButton *button = new QRadioButton(fileName.relative());
        button->setProperty("data", QVariant::fromValue(fileName));
        lay->addWidget(button);
        if (first) {
            button->setChecked(true);
            first = false;
        }
        QToolButton *details = new QToolButton;
        details->setText(i18nc("i for info", "i"));
        details->installEventFilter(this);
        details->setProperty("data", QVariant::fromValue(fileName));
        lay->addWidget(details);
        m_buttons.append(button);
    }
    rightSideLayout->addStretch(1);

    QFrame *line = new QFrame;
    line->setFrameStyle(QFrame::HLine);
    topLayout->addWidget(line);

    const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(files.first());
    const int angle = info->angle();
    ImageManager::ImageRequest *request = new ImageManager::ImageRequest(files.first(), QSize(300, 300), angle, this);
    ImageManager::AsyncLoader::instance()->load(request);
}

void DuplicateMatch::pixmapLoaded(ImageManager::ImageRequest * /*request*/, const QImage &image)
{
    m_image->setPixmap(QPixmap::fromImage(image));
}

void DuplicateMatch::setSelected(bool b)
{
    m_merge->setChecked(b);
}

bool DuplicateMatch::selected() const
{
    return m_merge->isChecked();
}

void DuplicateMatch::execute(Utilities::DeleteMethod method)
{
    if (!m_merge->isChecked())
        return;

    DB::FileName destination;
    Q_FOREACH (QRadioButton *button, m_buttons) {
        if (button->isChecked()) {
            destination = button->property("data").value<DB::FileName>();
            break;
        }
    }

    DB::FileNameList deleteList, dupList;
    Q_FOREACH (QRadioButton *button, m_buttons) {
        if (button->isChecked())
            continue;
        DB::FileName fileName = button->property("data").value<DB::FileName>();
        DB::ImageDB::instance()->copyData(fileName, destination);
        // can we safely delete the file?
        if (fileName != destination)
            deleteList.append(fileName);
        else
            dupList.append(fileName);
    }

    Utilities::DeleteFiles::deleteFiles(deleteList, method);
    // remove duplicate DB-entries without removing or blocking the file:
    DB::ImageDB::instance()->deleteList(dupList);
}

bool DuplicateMatch::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::Enter)
        return false;

    QToolButton *but;
    if (!(but = qobject_cast<QToolButton *>(obj)))
        return false;

    const DB::FileName fileName = but->property("data").value<DB::FileName>();
    MergeToolTip::instance()->requestToolTip(fileName);
    return false;
}

} // namespace MainWindow

// vi:expandtab:tabstop=4 shiftwidth=4:
