/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef OPTIONMATCHER_H
#define OPTIONMATCHER_H
#include <qvaluelist.h>
#include <qstringlist.h>
#include "imageinfoptr.h"
class ImageInfo;

/**
   Base class for components in the state machine for image matching.
*/
class OptionMatcher
{
public:
    virtual bool eval( ImageInfoPtr ) = 0;
    virtual ~OptionMatcher() {}
    virtual void debug( int level ) const = 0;
#ifdef TEMPORARILY_REMOVED
    virtual OptionMatcher* normalize() = 0;
    virtual OptionMatcher* clone() = 0;
#endif

protected:
    QString spaces(int level ) const;
};

class OptionSimpleMatcher :public OptionMatcher
{
public:
    QString _category;
    bool _sign;
};

class OptionValueMatcher :public OptionSimpleMatcher
{
public:
    OptionValueMatcher( const QString& category, const QString& value, bool sign );
    virtual bool eval( ImageInfoPtr );
    virtual void debug( int level ) const;
#ifdef TEMPORARILY_REMOVED
    virtual OptionMatcher* normalize();
    virtual OptionMatcher* clone();
#endif

    QString _option;
};


class OptionEmptyMatcher :public OptionSimpleMatcher
{
public:
    OptionEmptyMatcher( const QString& category, bool sign );
    virtual bool eval( ImageInfoPtr info );
    virtual void debug( int level ) const;
#ifdef TEMPORARILY_REMOVED
    virtual OptionMatcher* normalize();
    virtual OptionMatcher* clone();
#endif

};

class OptionContainerMatcher :public OptionMatcher
{
public:
    void addElement( OptionMatcher* );
    ~OptionContainerMatcher();
    virtual void debug( int level ) const;
#ifdef TEMPORARILY_REMOVED
    void clone( OptionContainerMatcher* newMatcher );
#endif

    QValueList<OptionMatcher*> _elements;
};

class OptionAndMatcher :public OptionContainerMatcher
{
public:
    virtual bool eval( ImageInfoPtr );
    virtual void debug( int level ) const;
#ifdef TEMPORARILY_REMOVED
    virtual OptionMatcher* normalize();
    OptionMatcher* normalizeTwo( OptionMatcher*, OptionMatcher* );
    virtual OptionMatcher* clone();
#endif
};



class OptionOrMatcher :public OptionContainerMatcher
{
public:
    virtual bool eval( ImageInfoPtr );
    virtual void debug( int level ) const;
#ifdef TEMPORARILY_REMOVED
    virtual OptionMatcher* normalize();
    virtual OptionMatcher* clone();
#endif
};

#endif /* OPTIONMATCHER_H */

