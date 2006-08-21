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

#ifndef SQLSETTINGSWIDGET_H
#define SQLSETTINGSWIDGET_H

#include <qwidget.h>
#include <kexidb/drivermanager.h>

namespace KexiDB { class ConnectionData; }
class QLabel;
class QComboBox;
class QWidgetStack;
class QSpinBox;
class KURLRequester;
class KLineEdit;
class KPasswordEdit;


class SQLSettingsWidget: public QWidget
{
    Q_OBJECT

public:
    SQLSettingsWidget(QWidget* parent=0, const char* name=0, WFlags fl=0);
    ~SQLSettingsWidget();

    QStringList availableDrivers() const;
    bool setDriver(const QString& driver);
    void setFile(const QString& filename);
    void setServerParameters(const QString& host, uint port,
                             const QString& database,
                             const QString& username);
    bool hasSettings() const;
    void getSettings(KexiDB::ConnectionData& connectionData,
                     QString& databaseName) const;

public slots:
    void reloadDriverList();

protected:
    QLabel* _errorLabel;
    QLabel* _driverLabel;
    QComboBox* _driverCombo;
    QWidgetStack* _widgetStack;
    QLabel* _fileLabel;
    KURLRequester* _fileLine;
    QLabel* _hostLabel;
    KLineEdit* _hostLine;
    QLabel* _portLabel;
    QSpinBox* _portSpin;
    QLabel* _dbNameLabel;
    KLineEdit* _dbNameLine;
    QLabel* _usernameLabel;
    KLineEdit* _usernameLine;
    QLabel* _passwordLabel;
    KPasswordEdit* _passwordLine;

    mutable KexiDB::DriverManager _driverManager;

protected slots:
    virtual void languageChange();
    void showOptionsOfSelectedDriver();

private:
    enum StackPage { ErrorPage, FileSettingsPage, ServerSettingsPage };
};

#endif /* SQLSETTINGSWIDGET_H */
