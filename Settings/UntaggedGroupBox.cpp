/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "UntaggedGroupBox.h"
#include "SettingsData.h"
#include <QDebug>
#include <klocale.h>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <DB/ImageDB.h>
#include "DB/CategoryCollection.h"

Settings::UntaggedGroupBox::UntaggedGroupBox( QWidget* parent )
    : QGroupBox( i18n("Untagged Images"), parent )
{


    QGridLayout* grid = new QGridLayout(this);
    int row = -1;

    QLabel* label = new QLabel( i18n("Category:" ) );
    grid->addWidget( label, ++row, 0 );

    m_category = new QComboBox;
    grid->addWidget( m_category, row, 1 );
    connect( m_category, SIGNAL(currentIndexChanged(int)), this, SLOT(populateTagsCombo()) );

    label = new QLabel( i18n("Tag:") );
    grid->addWidget( label, ++row, 0 );

    m_tag = new QComboBox;
    grid->addWidget( m_tag, row, 1 );
    m_tag->setEditable(true);

    grid->setColumnStretch(1,1);
}

void Settings::UntaggedGroupBox::populateCategoryComboBox()
{
    m_category->clear();
    m_category->addItem( i18n("None Selected") );
    Q_FOREACH( DB::CategoryPtr category, DB::ImageDB::instance()->categoryCollection()->categories() ) {
        if (!category->isSpecialCategory() )
            m_category->addItem( category->text(), category->name() );
    }
}

void Settings::UntaggedGroupBox::populateTagsCombo()
{
    m_tag->clear();
     const QString currentCategory = m_category->itemData(m_category->currentIndex() ).value<QString>();
    if ( currentCategory.isEmpty() )
        m_tag->setEnabled(false);
    else {
        m_tag->setEnabled(true);
        const QStringList items = DB::ImageDB::instance()->categoryCollection()->categoryForName( currentCategory )->items();
        m_tag->addItems( items );
    }
}

void Settings::UntaggedGroupBox::loadSettings( Settings::SettingsData* opt )
{
    populateCategoryComboBox();

    const QString category = opt->untaggedCategory();
    const QString tag = opt->untaggedTag();

    int categoryIndex = m_category->findData( category );
    if ( categoryIndex == -1 )
        categoryIndex = 0;

    m_category->setCurrentIndex( categoryIndex );
    populateTagsCombo();

    if ( categoryIndex != 0 ) {
        int tagIndex = m_tag->findText( tag );
        if ( tagIndex == -1 ) {
            m_tag->addItem( tag );
            tagIndex = m_tag->findText( tag );
            Q_ASSERT( tagIndex != -1 );
        }
        m_tag->setCurrentIndex( tagIndex );
    }


}

void Settings::UntaggedGroupBox::saveSettings( Settings::SettingsData* opt )
{
    const QString category = m_category->itemData(m_category->currentIndex() ).value<QString>();;
    if ( !category.isEmpty() ) {
        opt->setUntaggedCategory( category );
        opt->setUntaggedTag( m_tag->currentText() );
    } else {
        opt->setUntaggedCategory(QString());
        opt->setUntaggedTag(QString());
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
