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

