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

class MemberMap {
public:
    QStringList groups( const QString& optionGroup );
    void deleteGroup( const QString& optionGroup, const QString& name );
    QStringList members( const QString& optionGroup, const QString& memberGroup );
    void setMembers( const QString& optionGroup, const QString& memberGroup, const QStringList& members );
    QDomElement save( QDomDocument doc );
    bool isEmpty() const;
    void load( const QDomElement& );
    bool isGroup( const QString& optionGroup, const QString& memberGroup );

private:
    QMap<QString, QMap<QString,QStringList> > _members;
    friend class GroupCounter;
};


class Options :public QObject {
    Q_OBJECT

public:
    static Options* instance();
    static void setup( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory );

    void setThumbSize( int );
    int thumbSize() const;

    void setMaxImages( int );
    int maxImages() const;

    void setViewerSize( int width, int height );
    QSize viewerSize() const;

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

    // -------------------------------------------------- Member Groups
    MemberMap memberMap();
    void setMemberMap( const MemberMap& );

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

    void save( QDomElement top );

    QString imageDirectory() const;

    QString HTMLBaseDir() const;
    void setHTMLBaseDir( const QString& dir );

    QString HTMLBaseURL() const;
    void setHTMLBaseURL( const QString& dir );

    void saveConfigWindowLayout( ImageConfig* );
    void loadConfigWindowLayout( ImageConfig* );

signals:
    void optionGroupsChanged();
    void changed();

private:
    Options( const QDomElement& config, const QDomElement& options, const QDomElement& configWindowSetup, const QDomElement& memberGroups, const QString& imageDirectory  );
    static Options* _instance;

    int _thumbSize,  _imageCacheSize;
    TimeStampTrust _tTimeStamps;
    int _autoSave, _maxImages;
    bool _trustTimeStamps, _markNew, _hasAskedAboutTimeStamps, _ensureImageWindowsOnScreen;
    QMap<QString, QStringList> _options;
    QMap<QString,OptionGroupInfo> _optionGroups;
    QString _imageDirectory, _htmlBaseDir, _htmlBaseURL;

    Position _infoBoxPosition;
    bool _showInfoBox, _showDrawings, _showDescription, _showDate;
    QDomElement _configDock;

    QSize _viewerSize;

    MemberMap _members;
};

#endif /* OPTIONS_H */

