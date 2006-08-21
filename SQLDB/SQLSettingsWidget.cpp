/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

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

#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <kurlrequester.h>
#include <qspinbox.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kfiledialog.h>
#include <kpassdlg.h>
#include <klocale.h>
#include <kexidb/connectiondata.h>

SQLSettingsWidget::SQLSettingsWidget(QWidget* parent, const char* name, WFlags fl):
    QWidget(parent, name, fl)
{
    QBoxLayout* topLayout = new QVBoxLayout(this, 6);

    QHBoxLayout* drvSelLayout = new QHBoxLayout(topLayout, 6);
    _driverLabel = new QLabel(this);
    drvSelLayout->addWidget(_driverLabel);
    _driverCombo = new QComboBox(false, this);
    reloadDriverList();
    drvSelLayout->addWidget(_driverCombo);
    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                          QSizePolicy::Minimum);
    drvSelLayout->addItem(spacer);


    _widgetStack = new QWidgetStack(this);
    _widgetStack->setMargin(0);


    // Page 0, error information
    QWidget* stackPage = new QWidget(_widgetStack);
    QVBoxLayout* stackPage0Layout = new QVBoxLayout(stackPage, 0, 6);

    _errorLabel = new QLabel(stackPage);
    stackPage0Layout->addWidget(_errorLabel);

    spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                             QSizePolicy::Expanding);
    stackPage0Layout->addItem(spacer);

    _widgetStack->addWidget(stackPage, ErrorPage);


    // Page 1, file based database settings
    stackPage = new QWidget(_widgetStack);
    QVBoxLayout* stackPage1Layout = new QVBoxLayout(stackPage, 0, 6);

    spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                             QSizePolicy::Expanding);
    stackPage1Layout->addItem(spacer);

    QHBoxLayout* fileLayout = new QHBoxLayout(stackPage1Layout, 6);
    _fileLabel = new QLabel(stackPage);
    fileLayout->addWidget(_fileLabel);
    _fileLine = new KURLRequester(stackPage);
    _fileLine->setMinimumWidth(250);
    _fileLine->setMode(KFile::File | KFile::LocalOnly);
    _fileLine->fileDialog()->setOperationMode(KFileDialog::Saving);
    fileLayout->addWidget(_fileLine);

    spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                             QSizePolicy::Expanding);
    stackPage1Layout->addItem(spacer);

    _widgetStack->addWidget(stackPage, FileSettingsPage);


    // Page 2, server based database settings
    stackPage = new QWidget(_widgetStack);
    QGridLayout* stackPage2Layout = new QGridLayout(stackPage, 1, 1, 0, 6);

    _hostLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_hostLabel, 0, 0);

    _hostLine = new KLineEdit(stackPage);
    stackPage2Layout->addWidget(_hostLine, 0, 1);

    _portLabel = new QLabel(stackPage);
    stackPage2Layout->addWidget(_portLabel, 1, 0);

    _portSpin = new QSpinBox(stackPage);
    _portSpin->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,
                                           QSizePolicy::Fixed));
    _portSpin->setMaxValue(65535);
    stackPage2Layout->addWidget(_portSpin, 1, 1);

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

    _passwordLine = new KPasswordEdit(stackPage);
    stackPage2Layout->addWidget(_passwordLine, 4, 1);

    _widgetStack->addWidget(stackPage, ServerSettingsPage);


    topLayout->addWidget(_widgetStack);


    languageChange();

    resize(sizeHint());

    showOptionsOfSelectedDriver();

    connect(_driverCombo, SIGNAL(activated(int)),
            this, SLOT(showOptionsOfSelectedDriver()));
}

SQLSettingsWidget::~SQLSettingsWidget()
{
}

QStringList SQLSettingsWidget::SQLSettingsWidget::availableDrivers() const
{
    return _driverManager.driverNames();
}

bool SQLSettingsWidget::setDriver(const QString& driver)
{
    for (int i = 0; i < _driverCombo->count(); ++i) {
        if (_driverCombo->text(i) == driver) {
            _driverCombo->setCurrentItem(i);
            showOptionsOfSelectedDriver();
            return true;
        }
    }
    return false;
}

void SQLSettingsWidget::setFile(const QString& filename)
{
    _fileLine->setURL(filename);
}

void SQLSettingsWidget::setServerParameters(const QString& host, uint port,
                                            const QString& database,
                                            const QString& username)
{
    _hostLine->setText(host);
    _portSpin->setValue(port);
    _dbNameLine->setText(database);
    _usernameLine->setText(username);
}


bool SQLSettingsWidget::hasSettings() const
{
    KexiDB::Driver::Info driverInfo =
        _driverManager.driverInfo(_driverCombo->currentText());
    if (driverInfo.name.isEmpty())
        return false;
    if (driverInfo.fileBased)
        return !_fileLine->url().isEmpty();
    else
        return !_dbNameLabel->text().isEmpty();
}

void SQLSettingsWidget::getSettings(KexiDB::ConnectionData& connectionData,
                                    QString& databaseName) const
{
    KexiDB::Driver::Info driverInfo =
        _driverManager.driverInfo(_driverCombo->currentText());
    if (driverInfo.name.isEmpty())
        return;
    connectionData.driverName = _driverCombo->currentText();
    if (driverInfo.fileBased) {
        databaseName = _fileLine->url();
        connectionData.setFileName(databaseName);
    }
    else {
        connectionData.hostName = _hostLine->text();
        connectionData.port = _portSpin->value();
        databaseName = _dbNameLine->text();
        connectionData.userName = _usernameLine->text();
        connectionData.password = _passwordLine->text();
    }
}

void SQLSettingsWidget::reloadDriverList()
{
    _driverCombo->clear();
    QStringList drivers = availableDrivers();
    for (QStringList::const_iterator i = drivers.begin();
         i != drivers.end(); ++i) {
        // SQLite2 is not supported
        if (*i != QString::fromLatin1("SQLite2")) {
            _driverCombo->insertItem(*i);
            // SQLite3 is the default driver, so select it
            if (*i == QString::fromLatin1("SQLite3"))
                _driverCombo->setCurrentItem(_driverCombo->count() - 1);
        }
    }
}

void SQLSettingsWidget::languageChange()
{
    setCaption(i18n("SQL Database Settings"));
    _errorLabel->setText(i18n("No SQL database drivers found."));
    _driverLabel->setText(i18n("Database driver:"));
    _fileLabel->setText(i18n("Database file:"));
    _fileLine->setFilter("*.db|" + i18n("KPhotoAlbum database files (*.db)"));
    _hostLabel->setText(i18n("Server address:"));
    _portLabel->setText(i18n("Server port:"));
    _portSpin->setSpecialValueText(i18n("Default"));
    _dbNameLabel->setText(i18n("Database name:"));
    _usernameLabel->setText(i18n("Username:"));
    _passwordLabel->setText(i18n("Password:"));
}

void SQLSettingsWidget::showOptionsOfSelectedDriver()
{
    KexiDB::Driver::Info driverInfo =
        _driverManager.driverInfo(_driverCombo->currentText());
    if (driverInfo.name.isEmpty())
        _widgetStack->raiseWidget(ErrorPage);
    else
        if (driverInfo.fileBased)
            _widgetStack->raiseWidget(FileSettingsPage);
        else
            _widgetStack->raiseWidget(ServerSettingsPage);
}
