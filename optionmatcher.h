#ifndef OPTIONMATCHER_H
#define OPTIONMATCHER_H
#include <qvaluelist.h>
#include "imageinfo.h"

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
    OptionValueMatcher( const QString& optionGroup, const QString& option );
    virtual bool eval( ImageInfo* );
    virtual OptionMatcher* optimize();

private:
    QString _optionGroup;
    QString _option;
};


class OptionEmptyMatcher :public OptionMatcher
{
public:
    OptionEmptyMatcher( const QString& optionGroup );
    virtual bool eval( ImageInfo* info );
    virtual OptionMatcher* optimize();

private:
    QString _optionGroup;
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

