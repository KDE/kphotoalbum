#ifndef CATEGORYIMAGECONFIG_H
#define CATEGORYIMAGECONFIG_H

#include <kdialogbase.h>
#include <qimage.h>
class QComboBox;
class QLabel;

class CategoryImageConfig :public KDialogBase {
    Q_OBJECT

public:
    static CategoryImageConfig* instance();
    void setCurrentImage( const QImage& image );
    void show();

protected slots:
    void groupChanged();
    void memberChanged();
    void slotSet();

protected:
    QString currentGroup();

private:
    static CategoryImageConfig* _instance;
    CategoryImageConfig();
    QComboBox* _group;
    QComboBox* _member;
    QLabel* _current;
    QImage _image;
    QLabel* _imageLabel;
};


#endif /* CATEGORYIMAGECONFIG_H */

