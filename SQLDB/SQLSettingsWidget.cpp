/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "SQLSettingsWidget.h"
#include "DatabaseAddress.h"
#include "DriverManager.h"
#include "QueryErrors.h"
#include <kurlrequester.h>
#include <klineedit.h>
#include <kpassworddialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <QLabel>
#include <QComboBox>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSpinBox>

using namespace SQLDB;

SQLSettingsWidget::SQLSettingsWidget(QWidget* parent, Qt::WindowFlags fl):
    QWidget(parent, fl),
    _lastErrorType(NoDrivers)
{
    QVBoxLayout* topLayout = new QVBoxLayout(this);

    QHBoxLayout* drvSelLayout = new QHBoxLayout;
    topLayout->addLayout(drvSelLayout);
    _driverLabel = new QLabel(this);
    drvSelLayout->addWidget(_driverLabel);
    _driverCombo = new QComboBox(this);
    _driverCombo->setEditable(false);
    drvSelLayout->addWidget(_driverCombo);
    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
    drvSelLayout->addItem(spacer);


    _widgetStack = new QStackedWidget(this);


    // Page 0, error information
    QWidget* stackPage = new QWidget(_widgetStack);
    QVBoxLayout* stackPage0Layout = new QVBoxLayout(stackPage);

    _errorLabel = new QLabel(stackPage);
    stackPage0Layout->addWidget(_errorLabel);

    spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                             QSizePolicy::Expanding);
    stackPage0Layout->addItem(spacer);

    _widgetStack->insertWidget(ErrorPage, stackPage);


    // Page 1, file based database settings
    stackPage = new QWidget(_widgetStack);
    QVBoxLayout* stackPage1Layout = new QVBoxLayout(stackPage);

    spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                             QSizePolicy::Expanding);
    stackPage1Layout->addItem(spacer);

    QHBoxLayout* fileLayout = new QHBoxLayout;
    stackPage1Layout->addLayout(fileLayout);
    _fileLabel = new QLabel(stackPage);
    fileLayout->addWidget(_fileLabel);
    _fileLine = new KUrlRequester(stackPage);
    _fileLine->setMinimumWidth(250);
    _fileLine->setMode(KFile::File | KFile::LocalOnly);
    _fileLine->fileDialog()->setOperationMode(KFileDialog::Saving);
    fileLayout->addWidget(_fileLine);

    spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                             QSizePolicy::Expanding);
    stackPage1Layout->addItem(spacer);

    _widgetStack->insertWidget(FileSettingsPage, stackPage);


    // Page 2, server based database settings
    stackPage = new QWidget(_widgetStack);
    QGridLayout* stackPage2Layout = new QGridLayout(stackPage);

    _hostLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_hostLabel, 0, 0);
    _hostLine = new KLineEdit(stackPage);
    stackPage2Layout->addWidget(_hostLine, 0, 1);

    _portLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_portLabel, 1, 0);
    QHBoxLayout* portLayout = new QHBoxLayout;
    stackPage2Layout->addLayout(portLayout, 1, 1);
    _portSpin = new QSpinBox(stackPage);
    _portSpin->setMaximum(65535);
    portLayout->addWidget(_portSpin, 1, Qt::AlignLeft);
    spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                             QSizePolicy::Minimum);
    portLayout->addItem(spacer);

    _dbNameLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_dbNameLabel, 2, 0);
    _dbNameLine = new KLineEdit(stackPage);
    stackPage2Layout->addWidget(_dbNameLine, 2, 1);

    _usernameLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_usernameLabel, 3, 0);
    _usernameLine = new KLineEdit(stackPage);
    stackPage2Layout->addWidget(_usernameLine, 3, 1);

    _passwordLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_passwordLabel, 4, 0);
    _passwordLine = new KLineEdit(stackPage);
    _passwordLine->setPasswordMode(true);
    stackPage2Layout->addWidget(_passwordLine, 4, 1);

    _widgetStack->insertWidget(ServerSettingsPage, stackPage);


    topLayout->addWidget(_widgetStack);


    languageChange();

    resize(sizeHint());

    reloadDriverList();

    // Try to select the default driver
    selectDriver(QString::fromLatin1("SQLite3"));

    showOptionsOfSelectedDriver();

    connect(_driverCombo, SIGNAL(activated(int)),
            this, SLOT(showOptionsOfSelectedDriver()));
    connect(_driverCombo, SIGNAL(activated(const QString&)),
            this, SIGNAL(driverSelectionChanged(const QString&)));
    connect(_passwordLine, SIGNAL(textChanged(const QString&)),
            this, SIGNAL(passwordChanged(const QString&)));
}

QStringList SQLSettingsWidget::availableDrivers() const
{
    return DriverManager::instance().driverNames();
}

bool SQLSettingsWidget::hasSettings() const
{
    bool fileBased;

    try {
        DriverInfo driverInfo
            (DriverManager::instance().
             getDriverInfo(_driverCombo->currentText()));
        fileBased = driverInfo.isFileBased();
    }
    catch (DriverNotFoundError&) {
        return false;
    }
    if (fileBased)
        return !_fileLine->url().isEmpty();
    else
        return !_dbNameLabel->text().isEmpty();
}

DatabaseAddress SQLSettingsWidget::getSettings() const
{
    DatabaseAddress dbAddr;
    QString databaseName;

    try {
        DriverInfo driverInfo
            (DriverManager::instance().
             getDriverInfo(_driverCombo->currentText()));
        dbAddr.setDriverName(driverInfo.name());
        dbAddr.setFileBased(driverInfo.isFileBased());
    }
    catch (DriverNotFoundError&) {
        return DatabaseAddress();
    }

    if (dbAddr.isFileBased()) {
        dbAddr.setDatabaseName(_fileLine->url().path());
    }
    else {
        dbAddr.setHost(_hostLine->text(), _portSpin->value());
        dbAddr.setDatabaseName(_dbNameLine->text());
        dbAddr.setUserName(_usernameLine->text());
        dbAddr.setPassword(_passwordLine->text());
    }

    return dbAddr;
}

void SQLSettingsWidget::setSettings(const DatabaseAddress& address)
{
    selectDriver(address.driverName());
    showOptionsOfSelectedDriver();
    QString fileName;
    QString databaseName;
    if (address.isFileBased())
        fileName = address.databaseName();
    else
        databaseName = address.databaseName();

    _fileLine->setUrl(fileName);
    _hostLine->setText(address.hostName());
    _portSpin->setValue(address.port());
    _dbNameLine->setText(databaseName);
    _usernameLine->setText(address.userName());
    _passwordLine->setText(address.password());
}

void SQLSettingsWidget::reloadDriverList()
{
    _driverCombo->clear();
    const QStringList drivers = availableDrivers();
    for (QStringList::const_iterator i = drivers.constBegin();
         i != drivers.constEnd(); ++i) {
        _driverCombo->addItem(*i);
    }
    if (_driverCombo->count() == 0)
        _driverCombo->setEnabled(false);
}

void SQLSettingsWidget::selectDriver(const QString& driver)
{
    for (int i = 0; i < _driverCombo->count(); ++i) {
        if (_driverCombo->itemText(i) == driver) {
            _driverCombo->setCurrentIndex(i);
            showOptionsOfSelectedDriver();
            break;
        }
    }
}

void SQLSettingsWidget::languageChange()
{
    setError(_lastErrorType);
    _driverLabel->setText(i18n("Database driver:"));
    _fileLabel->setText(i18n("Database file:"));
    _fileLine->setFilter(QString::fromLatin1("*.db|") + i18n("KPhotoAlbum database files (*.db)"));
    _hostLabel->setText(i18n("Server address:"));
    _portLabel->setText(i18n("Server port:"));
    _portSpin->setSpecialValueText(i18n("Default"));
    _dbNameLabel->setText(i18n("Database name:"));
    _usernameLabel->setText(i18n("Username:"));
    _passwordLabel->setText(i18n("Password:"));
}

void SQLSettingsWidget::showOptionsOfSelectedDriver()
{
    if (_driverCombo->count() == 0) {
        setError(NoDrivers);
        _widgetStack->setCurrentIndex(ErrorPage);
        return;
    }

    bool fileBased;

    try {
        DriverInfo driverInfo
            (DriverManager::instance().
             getDriverInfo(_driverCombo->currentText()));
        fileBased = driverInfo.isFileBased();
    }
    catch (DriverNotFoundError&) {
        setError(InvalidDriver);
        _widgetStack->setCurrentIndex(ErrorPage);
        return;
    }

    if (fileBased)
        _widgetStack->setCurrentIndex(FileSettingsPage);
    else
        _widgetStack->setCurrentIndex(ServerSettingsPage);
}

void SQLSettingsWidget::setError(ErrorType errorType)
{
    switch (errorType) {
    case NoDrivers:
        _errorLabel->setText(i18n("No SQL database drivers found."));
        break;

    case InvalidDriver:
        _errorLabel->setText(i18n("Invalid driver."));
        break;
    }
    _lastErrorType = errorType;
}

#include "SQLSettingsWidget.moc"
