/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include <KoGlobal.h>
#include <kdebug.h>
#include <qfont.h>
#include <qfontinfo.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kimageio.h>
#include <kiconloader.h>
#include <kstandarddirs.h>


KoGlobal* KoGlobal::s_global = 0L;
static KStaticDeleter<KoGlobal> sdg;

KoGlobal* KoGlobal::self()
{
    if ( !s_global )
        sdg.setObject( s_global, new KoGlobal );
    return s_global;
}

KoGlobal::KoGlobal()
    : m_pointSize( -1 ), m_kofficeConfig( 0L )
{
    // Install the libkoffice* translations
    KGlobal::locale()->insertCatalogue("koffice");

    KImageIO::registerFormats();

    // Tell KStandardDirs about the koffice prefix
    KGlobal::dirs()->addPrefix(PREFIX);

    // Tell the iconloader about share/apps/koffice/icons
    KGlobal::iconLoader()->addAppDir("koffice");

    // Another way to get the DPI of the display would be QPaintDeviceMetrics,
    // but we have no widget here (and moving this to KoView wouldn't allow
    // using this from the document easily).
#ifdef Q_WS_X11
    m_dpiX = QPaintDevice::x11AppDpiX();
    m_dpiY = QPaintDevice::x11AppDpiY();
#else
    m_dpiX = 75;
    m_dpiY = 75;
#endif
}

KoGlobal::~KoGlobal()
{
    delete m_kofficeConfig;
}

QFont KoGlobal::_defaultFont()
{
    QFont font = KGlobalSettings::generalFont();
    // we have to use QFontInfo, in case the font was specified with a pixel size
    if ( font.pointSize() == -1 )
    {
        // cache size into m_pointSize, since QFontInfo loads the font -> slow
        if ( m_pointSize == -1 )
            m_pointSize = QFontInfo(font).pointSize();
        Q_ASSERT( m_pointSize != -1 );
        font.setPointSize( m_pointSize );
    }
    //kdDebug()<<k_funcinfo<<"QFontInfo(font).pointSize() :"<<QFontInfo(font).pointSize()<<endl;
    //kdDebug()<<k_funcinfo<<"font.name() :"<<font.family ()<<endl;
    return font;
}

QStringList KoGlobal::_listOfLanguageTags()
{
    if ( m_langMap.isEmpty() )
        createListOfLanguages();
    return m_langMap.values();
}

QStringList KoGlobal::_listOfLanguages()
{
    if ( m_langMap.empty() )
        createListOfLanguages();
    return m_langMap.keys();
}

void KoGlobal::createListOfLanguages()
{
    KConfig config( "all_languages", true, false, "locale" );
    // Note that we could also use KLocale::allLanguagesTwoAlpha

    QMap<QString, bool> seenLanguages;
    const QStringList langlist = config.groupList();
    for ( QStringList::ConstIterator itall = langlist.begin();
          itall != langlist.end(); ++itall )
    {
        const QString tag = *itall;
        config.setGroup( tag );
        const QString name = config.readEntry("Name", tag);
        // e.g. name is "French" and tag is "fr"

        // The QMap does the sorting on the display-name, so that
        // comboboxes are sorted.
        m_langMap.insert( name, tag );

        seenLanguages.insert( tag, true );
    }

    // Also take a look at the installed translations.
    // Many of them are already in all_languages but all_languages doesn't
    // currently have en_GB or en_US etc.

    const QStringList translationList = KGlobal::dirs()->findAllResources("locale",
                                                            QString::fromLatin1("*/entry.desktop"));
    for ( QStringList::ConstIterator it = translationList.begin();
          it != translationList.end(); ++it )
    {
        // Extract the language tag from the directory name
        QString tag = *it;
        int index = tag.findRev('/');
        tag = tag.left(index);
        index = tag.findRev('/');
        tag = tag.mid(index+1);

        if ( seenLanguages.find( tag ) == seenLanguages.end() ) {
            KSimpleConfig entry(*it);
            entry.setGroup("KCM Locale");

            const QString name = entry.readEntry("Name", tag);
            // e.g. name is "US English" and tag is "en_US"
            m_langMap.insert( name, tag );

            // enable this if writing a third way of finding languages below
            //seenLanguages.insert( tag, true );
        }

    }

    // #### We also might not have an entry for a language where spellchecking is supported,
    //      but no KDE translation is available, like fr_CA.
    // How to add them?
}

QString KoGlobal::tagOfLanguage( const QString & _lang)
{
    const LanguageMap& map = self()->m_langMap;
    QMap<QString,QString>::ConstIterator it = map.find( _lang );
    if ( it != map.end() )
        return *it;
    return QString::null;
}

QString KoGlobal::languageFromTag( const QString &langTag )
{
    const LanguageMap& map = self()->m_langMap;
    QMap<QString,QString>::ConstIterator it = map.begin();
    const QMap<QString,QString>::ConstIterator end = map.end();
    for ( ; it != end; ++it )
        if ( it.data() == langTag )
            return it.key();

    // Language code not found. Better return the code (tag) than nothing.
    return langTag;
}

KConfig* KoGlobal::_kofficeConfig()
{
    if ( !m_kofficeConfig ) {
        m_kofficeConfig = new KConfig( "kofficerc" );
    }
    return m_kofficeConfig;
}

void KoGlobal::setDPI( int x, int y )
{
    //kdDebug() << k_funcinfo << x << "," << y << endl;
    KoGlobal* s = self();
    s->m_dpiX = x;
    s->m_dpiY = y;
}
