#ifndef BOOLEANGUARD_H
#define BOOLEANGUARD_H

namespace Utilities
{

class BooleanGuard
{
public:
    BooleanGuard( bool& guard );
    ~BooleanGuard();
    bool canContinue();

private:
    bool& _guard;
    bool _iLocedIt;
};


}


#endif /* BOOLEANGUARD_H */

