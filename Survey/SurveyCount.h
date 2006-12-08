/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef SURVEY_COUNT_H
#define SURVEY_COUNT_H

#include "Question.h"
class QSpinBox;
class QLineEdit;

namespace Survey
{

class SurveyCountQuestion : public Survey::Question {

public:
    SurveyCountQuestion(  const QString& id, const QString& title, Survey::SurveyDialog* parent );

protected:
    virtual void save( QDomElement& );
    virtual void load( QDomElement& );

private:
    QSpinBox* _imageCount;
    QSpinBox* _scanned;
};

}

#endif /* SURVEY_COUNT_H */

