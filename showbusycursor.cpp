#include "showbusycursor.h"
#include <qapplication.h>
#include <qcursor.h>

ShowBusyCursor::ShowBusyCursor()
{
    qApp->setOverrideCursor( QCursor( Qt::WaitCursor ) );
}

ShowBusyCursor::~ShowBusyCursor()
{
    qApp->restoreOverrideCursor();
}
