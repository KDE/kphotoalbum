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

#ifndef SURVEY_H
#define SURVEY_H

class SurveyPrivate;
#include <qdialog.h>

namespace Survey {
    class Question;

    class SurveyDialog :public QDialog
    {
        Q_OBJECT
    public:
        SurveyDialog( QWidget* parent, const char* name = 0);
        ~SurveyDialog();
        void setFrontPage( QWidget* page );
        void setBackPage( QWidget* page );
        void setReceiver( const QString& emailAddress );
        void setSurveyVersion( int major, int minor );
        void exec();
        void possibleExecSurvey( int minInvocations = 50, int remindCount = 10 );

    protected:
        QWidget* createStackItem( Question*, int count );
        void go( int direction );
        void goToPage( int page );
        void setupFrontPage();
        void setupBackPage( int count );
        bool lastPage() const;
        void saveConfig( const QCString& xml );
        void readConfig();
        QCString configAsXML();

    private:
        friend class Question;
        void addQuestion( Question* );

    protected slots:
        void slotPrev();
        void slotNext();
        void slotDone();

    private:
        SurveyPrivate* d;
    };
}

#endif /* SURVEY_H */

