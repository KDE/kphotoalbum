#ifndef IMPORTMATCHER_H
#define IMPORTMATCHER_H

#include <qscrollview.h>
class QGridLayout;
class QComboBox;
class QCheckBox;

class OptionMatch  {
public:
    OptionMatch( const QString& optioin, const QStringList& myOptionList, QWidget* parent, QGridLayout* grid, int row );
    QCheckBox* _checkbox;
    QComboBox* _combobox;
};


class ImportMatcher :public QScrollView {
    Q_OBJECT

public:
    ImportMatcher( const QString& otherOptionGroup, const QString& myOptionGroup,
                   const QStringList& otherOptionList, const QStringList& myOptionList,
                   QWidget* parent, const char* name = 0 );

    QString _otherOptionGroup;
    QString _myOptionGroup;
    QValueList<OptionMatch*> _matchers;
};


#endif /* IMPORTMATCHER_H */

