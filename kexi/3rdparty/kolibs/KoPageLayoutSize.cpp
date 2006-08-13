/* This file is part of the KDE project
 * Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KoPageLayoutDia.h>
#include <KoPageLayoutSize.h>
#include <KoUnit.h>
#include <KoUnitWidgets.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>

KoPageLayoutSize::KoPageLayoutSize(QWidget *parent, const KoPageLayout& layout, KoUnit::Unit unit,const KoColumns& columns,  bool unitChooser, bool enableBorders)
    : QWidget(parent) {
    m_layout = layout;
    m_unit = unit;

    QGridLayout *grid1 = new QGridLayout( this, 5, 2, 0, KDialog::spacingHint() );
    if ( unitChooser ) {
        // ------------- unit _______________
        QWidget* unitFrame = new QWidget( this );
        grid1->addWidget( unitFrame, 0, 0, Qt::AlignLeft );
        QBoxLayout* unitLayout = new QHBoxLayout( unitFrame, KDialog::marginHint(), KDialog::spacingHint() );

        // label unit
        QLabel *lpgUnit = new QLabel( i18n( "Unit:" ), unitFrame );
        unitLayout->addWidget( lpgUnit, 0, Qt::AlignRight | Qt::AlignVCenter );

        // combo unit
        cpgUnit = new QComboBox( false, unitFrame, "cpgUnit" );
        lpgUnit->setBuddy( cpgUnit );
        cpgUnit->insertStringList( KoUnit::listOfUnitName() );
        cpgUnit->setCurrentItem( unit );
        unitLayout->addWidget( cpgUnit, 0, Qt::AlignLeft | Qt::AlignVCenter );
        connect( cpgUnit, SIGNAL( activated( int ) ), this, SLOT( setUnitInt( int ) ) );
    }
    else {
        QString str=KoUnit::unitDescription(unit);

        QLabel *lpgUnit = new QLabel( i18n("All values are given in %1.").arg(str), this );
        grid1->addWidget( lpgUnit, 0, 0, Qt::AlignLeft );
    }

    // -------------- page size -----------------
    QVGroupBox *formatFrame = new QVGroupBox( i18n( "Page Size" ), this );
    grid1->addWidget( formatFrame, 1, 0 );

    QHBox *formatPageSize = new QHBox( formatFrame );
    formatPageSize->setSpacing( KDialog::spacingHint() );

    // label page size
    QLabel *lpgFormat = new QLabel( i18n( "&Size:" ), formatPageSize );

    // combo size
    cpgFormat = new QComboBox( false, formatPageSize, "cpgFormat" );
    cpgFormat->insertStringList( KoPageFormat::allFormats() );
    lpgFormat->setBuddy( cpgFormat );
    connect( cpgFormat, SIGNAL( activated( int ) ), this, SLOT( formatChanged( int ) ) );

    // spacer
    formatPageSize->setStretchFactor( new QWidget( formatPageSize ), 10 );

    QHBox *formatCustomSize = new QHBox( formatFrame );
    formatCustomSize->setSpacing( KDialog::spacingHint() );

    // label width
    QLabel *lpgWidth = new QLabel( i18n( "&Width:" ), formatCustomSize );

    // linedit width
    epgWidth = new KoUnitDoubleSpinBox( formatCustomSize, "Width" );
    lpgWidth->setBuddy( epgWidth );
    if ( m_layout.format != PG_CUSTOM )
        epgWidth->setEnabled( false );
    connect( epgWidth, SIGNAL( valueChangedPt(double) ), this, SLOT( widthChanged(double) ) );

    // label height
    QLabel *lpgHeight = new QLabel( i18n( "&Height:" ), formatCustomSize );

    // linedit height
    epgHeight = new KoUnitDoubleSpinBox( formatCustomSize, "Height" );
    lpgHeight->setBuddy( epgHeight );
    if ( m_layout.format != PG_CUSTOM )
        epgHeight->setEnabled( false );
    connect( epgHeight, SIGNAL( valueChangedPt(double ) ), this, SLOT( heightChanged(double) ) );

    // --------------- orientation ---------------
    m_orientGroup = new QHButtonGroup( i18n( "Orientation" ), this );
    m_orientGroup->setInsideSpacing( KDialog::spacingHint() );
    grid1->addWidget( m_orientGroup, 2, 0 );

    QLabel* lbPortrait = new QLabel( m_orientGroup );
    lbPortrait->setPixmap( QPixmap( UserIcon( "koPortrait" ) ) );
    lbPortrait->setMaximumWidth( lbPortrait->pixmap()->width() );
    new QRadioButton( i18n("&Portrait"), m_orientGroup );

    QLabel* lbLandscape = new QLabel( m_orientGroup );
    lbLandscape->setPixmap( QPixmap( UserIcon( "koLandscape" ) ) );
    lbLandscape->setMaximumWidth( lbLandscape->pixmap()->width() );
    new QRadioButton( i18n("La&ndscape"), m_orientGroup );

    connect( m_orientGroup, SIGNAL (clicked (int)), this, SLOT( orientationChanged(int) ));

    // --------------- page margins ---------------
    QVGroupBox *marginsFrame = new QVGroupBox( i18n( "Margins" ), this );
    marginsFrame->setColumnLayout( 0, Qt::Vertical );
    marginsFrame->setMargin( KDialog::marginHint() );
    grid1->addWidget( marginsFrame, 3, 0 );

    QGridLayout *marginsLayout = new QGridLayout( marginsFrame->layout(), 3, 3,
       KDialog::spacingHint() );

    // left margin
    ebrLeft = new KoUnitDoubleSpinBox( marginsFrame, "Left" );
    marginsLayout->addWidget( ebrLeft, 1, 0 );
    connect( ebrLeft, SIGNAL( valueChangedPt( double ) ), this, SLOT( leftChanged( double ) ) );

    // right margin
    ebrRight = new KoUnitDoubleSpinBox( marginsFrame, "Right" );
    marginsLayout->addWidget( ebrRight, 1, 2 );
    connect( ebrRight, SIGNAL( valueChangedPt( double ) ), this, SLOT( rightChanged( double ) ) );

    // top margin
    ebrTop = new KoUnitDoubleSpinBox( marginsFrame, "Top" );
    marginsLayout->addWidget( ebrTop, 0, 1 , Qt::AlignCenter );
    connect( ebrTop, SIGNAL( valueChangedPt( double ) ), this, SLOT( topChanged( double ) ) );

    // bottom margin
    ebrBottom = new KoUnitDoubleSpinBox( marginsFrame, "Bottom" );
    marginsLayout->addWidget( ebrBottom, 2, 1, Qt::AlignCenter );
    connect( ebrBottom, SIGNAL( valueChangedPt( double ) ), this, SLOT( bottomChanged( double ) ) );

    // ------------- preview -----------
    pgPreview = new KoPagePreview( this, "Preview", m_layout );
    grid1->addMultiCellWidget( pgPreview, 1, 3, 1, 1 );

    // ------------- spacers -----------
    QWidget* spacer1 = new QWidget( this );
    QWidget* spacer2 = new QWidget( this );
    spacer1->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
       QSizePolicy::Expanding ) );
    spacer2->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
       QSizePolicy::Expanding ) );
    grid1->addWidget( spacer1, 4, 0 );
    grid1->addWidget( spacer2, 4, 1 );

    setValues();
    updatePreview();
    pgPreview->setPageColumns( columns );
    setEnableBorders(enableBorders);
}

void KoPageLayoutSize::setEnableBorders(bool on) {
    ebrLeft->setEnabled( on );
    ebrRight->setEnabled( on );
    ebrTop->setEnabled( on );
    ebrBottom->setEnabled( on );

    // update m_layout
    m_layout.ptLeft = on?ebrLeft->value():0;
    m_layout.ptRight = on?ebrRight->value():0;
    m_layout.ptTop = on?ebrTop->value():0;
    m_layout.ptBottom = on?ebrBottom->value():0;

    // use updated m_layout
    updatePreview();
    emit propertyChange(m_layout);
}

void KoPageLayoutSize::updatePreview() {
    pgPreview->setPageLayout( m_layout );
}

void KoPageLayoutSize::setValues() {
    // page format
    cpgFormat->setCurrentItem( m_layout.format );
    // orientation
    m_orientGroup->setButton( m_layout.orientation == PG_PORTRAIT ? 0: 1 );

    setUnit( m_unit );
    pgPreview->setPageLayout( m_layout );
}

void KoPageLayoutSize::setUnit( KoUnit::Unit unit ) {
    m_unit = unit;

    //setUnit always befor changeValue
    epgWidth->setUnit( m_unit );
    epgWidth->setMinMaxStep( 0, KoUnit::fromUserValue( 9999, m_unit ), KoUnit::fromUserValue( 0.01, m_unit ) );
    epgWidth->changeValue( m_layout.ptWidth );

    epgHeight->setUnit( m_unit );
    epgHeight->setMinMaxStep( 0, KoUnit::fromUserValue( 9999, m_unit ), KoUnit::fromUserValue( 0.01, m_unit ) );
    epgHeight->changeValue( m_layout.ptHeight );

    double dStep = KoUnit::fromUserValue( 0.2, m_unit );

    ebrLeft->setUnit( m_unit );
    ebrLeft->changeValue( m_layout.ptLeft );
    ebrLeft->setMinMaxStep( 0, m_layout.ptWidth, dStep );

    ebrRight->setUnit( m_unit );
    ebrRight->changeValue( m_layout.ptRight );
    ebrRight->setMinMaxStep( 0, m_layout.ptWidth, dStep );

    ebrTop->setUnit( m_unit );
    ebrTop->changeValue( m_layout.ptTop );
    ebrTop->setMinMaxStep( 0, m_layout.ptHeight, dStep );

    ebrBottom->setUnit( m_unit );
    ebrBottom->changeValue( m_layout.ptBottom );
    ebrBottom->setMinMaxStep( 0, m_layout.ptHeight, dStep );
    emit propertyChange(m_layout);
}

void KoPageLayoutSize::setUnitInt( int unit ) {
    setUnit((KoUnit::Unit)unit);
}

void KoPageLayoutSize::formatChanged( int format ) {
    if ( ( KoFormat )format == m_layout.format )
        return;
    m_layout.format = ( KoFormat )format;
    bool enable =  (KoFormat) format == PG_CUSTOM;
    epgWidth->setEnabled( enable );
    epgHeight->setEnabled( enable );

    if ( m_layout.format != PG_CUSTOM ) {
        m_layout.ptWidth = MM_TO_POINT( KoPageFormat::width(
                    m_layout.format, m_layout.orientation ) );
        m_layout.ptHeight = MM_TO_POINT( KoPageFormat::height(
                    m_layout.format, m_layout.orientation ) );
    }

    epgWidth->changeValue( m_layout.ptWidth );
    epgHeight->changeValue( m_layout.ptHeight );

    updatePreview( );
    emit propertyChange(m_layout);
}

void KoPageLayoutSize::orientationChanged(int which) {
    KoOrientation oldOrientation = m_layout.orientation;
    m_layout.orientation = which == 0 ? PG_PORTRAIT : PG_LANDSCAPE;

    // without this check, width & height would be swapped around (below)
    // even though the orientation has not changed
    if (m_layout.orientation == oldOrientation) return;

    m_layout.ptWidth = epgWidth->value();
    m_layout.ptHeight = epgHeight->value();
    m_layout.ptLeft = ebrLeft->value();
    m_layout.ptRight = ebrRight->value();
    m_layout.ptTop = ebrTop->value();
    m_layout.ptBottom = ebrBottom->value();

    // swap dimension and adjust margins
    qSwap( m_layout.ptWidth, m_layout.ptHeight );
    double tmp = m_layout.ptTop;
    m_layout.ptTop = m_layout.ptRight;
    m_layout.ptRight = m_layout.ptBottom;
    m_layout.ptBottom = m_layout.ptLeft;
    m_layout.ptLeft = tmp;

    setValues();
    updatePreview( );
    emit propertyChange(m_layout);
}

void KoPageLayoutSize::widthChanged(double width) {
    m_layout.ptWidth = width;
    updatePreview();
    emit propertyChange(m_layout);
}
void KoPageLayoutSize::heightChanged(double height) {
    m_layout.ptHeight = height;
    updatePreview( );
    emit propertyChange(m_layout);
}
void KoPageLayoutSize::leftChanged( double left ) {
    m_layout.ptLeft = left;
    updatePreview();
    emit propertyChange(m_layout);
}
void KoPageLayoutSize::rightChanged(double right) {
    m_layout.ptRight = right;
    updatePreview();
    emit propertyChange(m_layout);
}
void KoPageLayoutSize::topChanged(double top) {
    m_layout.ptTop = top;
    updatePreview();
    emit propertyChange(m_layout);
}
void KoPageLayoutSize::bottomChanged(double bottom) {
    m_layout.ptBottom = bottom;
    updatePreview();
    emit propertyChange(m_layout);
}

bool KoPageLayoutSize::queryClose() {
    if ( m_layout.ptLeft + m_layout.ptRight > m_layout.ptWidth ) {
        KMessageBox::error( this,
            i18n("The page width is smaller than the left and right margins."),
                            i18n("Page Layout Problem") );
        return false;
    }
    if ( m_layout.ptTop + m_layout.ptBottom > m_layout.ptHeight ) {
        KMessageBox::error( this,
            i18n("The page height is smaller than the top and bottom margins."),
                            i18n("Page Layout Problem") );
        return false;
    }
    return true;
}

void KoPageLayoutSize::setColumns(KoColumns &columns) {
    pgPreview->setPageColumns(columns);
}

#include <KoPageLayoutSize.moc>
