#include <ksplashscreen.h>
#ifndef MYSPLASHSCREEN_H
#define MYSPLASHSCREEN_H

class MySplashScreen :public KSplashScreen {
    Q_OBJECT

public:
    MySplashScreen();
    static MySplashScreen* instance();
    virtual bool close( bool alsoDelete );

private:
    static MySplashScreen* _instance;
};


#endif /* MYSPLASHSCREEN_H */

