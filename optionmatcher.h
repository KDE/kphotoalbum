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
class ImageInfo;

/**
   Base class for components in the state machine for image matching.
*/
class OptionMatcher
{
public:
    virtual bool eval( ImageInfo* ) = 0;
    virtual ~OptionMatcher() {}
    virtual OptionMatcher* optimize() = 0;
};

class OptionValueMatcher :public OptionMatcher
{
public:
    OptionValueMatcher( const QString& category, const QString& option );
    virtual bool eval( ImageInfo* );
    virtual OptionMatcher* optimize();

private:
    QString _category;
    QString _option;
};


class OptionEmptyMatcher :public OptionMatcher
{
public:
    OptionEmptyMatcher( const QString& category );
    virtual bool eval( ImageInfo* info );
    virtual OptionMatcher* optimize();

private:
    QString _category;
};

class OptionContainerMatcher :public OptionMatcher
{
public:
    virtual OptionMatcher* optimize();
    void addElement( OptionMatcher* );
    ~OptionContainerMatcher();

protected:
    QValueList<OptionMatcher*> _elements;
};

class OptionAndMatcher :public OptionContainerMatcher
{
public:
    virtual bool eval( ImageInfo* );
};



class OptionOrMatcher :public OptionContainerMatcher
{
public:
    virtual bool eval( ImageInfo* );
};



class OptionNotMatcher :public OptionMatcher
{
public:
    OptionNotMatcher( OptionMatcher* );
    virtual bool eval( ImageInfo* );
    virtual OptionMatcher* optimize();
private:
    OptionMatcher* _element;
};

#endif /* OPTIONMATCHER_H */

