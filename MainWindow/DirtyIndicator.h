#ifndef DIRTYINDICATOR_H
#define DIRTYINDICATOR_H

#include <qlabel.h>
#include <qpixmap.h>

namespace MainWindow
{
class Window;

class DirtyIndicator :public QLabel
{
    Q_OBJECT

public:
    static void markDirty();

signals:
    void dirty();

private:
    friend class Window;
    DirtyIndicator( QWidget* parent );
    void autoSaved();
    void saved();
    bool isSaveDirty() const;
    bool isAutoSaveDirty() const;

    QPixmap _dirtyPix;
    static bool _autoSaveDirty;
    static bool _saveDirty;
};

}

#endif /* DIRTYINDICATOR_H */

