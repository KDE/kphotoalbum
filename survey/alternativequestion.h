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

#ifndef ALTERNATIVEQUESTION_H
#define ALTERNATIVEQUESTION_H

#include <question.h>
#include <qvaluelist.h>
class QLineEdit;
class QButton;
class QButtonGroup;

namespace Survey {

    class AlternativeQuestion :public Question
    {
    public:
        enum Type {CheckBox, RadioButton };
        AlternativeQuestion( const QString& id, const QString& title, const QString& text,
                             const QString& question, const QStringList& questions, int otherCounts,
                             Type tp, SurveyDialog* parent );

    protected:
        virtual void save( QDomElement& );
        virtual void load( QDomElement& );

    private:
        QValueList<QButton*> _buttons;
        QValueList<QLineEdit*> _edits;
    };

    class RadioButtonQuestion :public AlternativeQuestion
    {
    public:
        RadioButtonQuestion( const QString& id, const QString& title, const QString& text,
                             const QString& question, const QStringList& questions, SurveyDialog* parent );
    };
}


#endif /* ALTERNATIVEQUESTION_H */

