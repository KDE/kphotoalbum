/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef OPTIONS_H
#define OPTIONS_H
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qobject.h>
#include <qdom.h>
class ImageConfig;

class Options :public QObject {
    Q_OBJECT

public:
    static Options* instance();
    static bool configFileExists();
    static void setConfFile( const QString& file );
    QString configFile() const;
    QString autoSaveFile() const;

    void setThumbSize( int );
    int thumbSize() const;

    void setMaxImages( int );
    int maxImages() const;

    // -------------------------------------------------- Options
    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QString& value );
    void removeOption( const QString& key, const QString& value );
    QStringList optionValue( const QString& key ) const;

    // -------------------------------------------------- Option Groups
    struct OptionGroupInfo
    {
        OptionGroupInfo() {}
        OptionGroupInfo( const QString& text, const QString& icon, bool show = true )
            : _text(text), _icon(icon), _show(show) {}
        QString _text;
        QString _icon;
        bool _show;
    };

    QStringList optionGroups() const;
    void addOptionGroup( const QString& name, const QString& label, const QString& icon );
    void deleteOptionGroup( const QString& name );
    void renameOptionGroup( const QString& oldName, const QString& newName );

    QString textForOptionGroup( const QString& name ) const;

    QPixmap iconForOptionGroup( const QString& name ) const;
    QString iconNameForOptionGroup( const QString& name ) const;
    void setIconForOptionGroup( const QString& name, const QString& icon );

    // -------------------------------------------------- Options for the Viewer
    enum Position { Bottom = 0, Top, Left, Right, TopLeft, TopRight, BottomLeft, BottomRight };
    bool showInfoBox() const;
    bool showDrawings() const;
    bool showDescription() const;
    bool showDate() const;
    bool showOption( const QString& ) const;

    void setShowInfoBox(bool b);
    void setShowDrawings(bool b);
    void setShowDescription(bool b);
    void setShowDate(bool b);
    void setShowOption( const QString& optionGroup, bool b );

    Position infoBoxPosition() const;
    void setInfoBoxPosition( Position pos );

    // -------------------------------------------------- misc
    enum TimeStampTrust {
        Always = 0,
        Ask = 1,
        Never = 2
    };

    bool trustTimeStamps();
    void setTTimeStamps( TimeStampTrust );
    TimeStampTrust tTimeStamps() const;

    void setAutoSave( int min );
    int autoSave() const;

    void save( const QString& fileName );
    bool isDirty() const { return _dirty; }

    QString imageDirectory() const;
    void setImageDirecotry( const QString& directory );

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    void saveConfigWindowLayout( ImageConfig* );
    void loadConfigWindowLayout( ImageConfig* );

signals:
    void optionGroupsChanged();

private:
    Options();
    ~Options() {};
    static Options* _instance;
    static QString _confFile;

    int _thumbSize,  _imageCacheSize;
    TimeStampTrust _tTimeStamps;
    int _autoSave, _maxImages;
    bool _trustTimeStamps, _markNew, _hasAskedAboutTimeStamps;
    QMap<QString, QStringList> _options;
    QMap<QString,OptionGroupInfo> _optionGroups;
    QString _imageDirectory, _htmlBaseDir, _htmlBaseURL;

    Position _infoBoxPosition;
    bool _showInfoBox, _showDrawings, _showDescription, _showDate;
    bool _dirty;
    QDomElement _configDock;
};

#endif /* OPTIONS_H */

