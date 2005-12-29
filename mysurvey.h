#ifndef MYSURVEY_H
#define MYSURVEY_H
#include "Survey/surveydialog.h"

class MySurvey : public Survey::SurveyDialog {
    Q_OBJECT

public:
    MySurvey( QWidget* parent, const char* name = 0 );
    virtual QSize sizeHint() const;
};


#endif /* MYSURVEY_H */

