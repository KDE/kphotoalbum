#ifndef EDITOR_H
#define EDITOR_H
#include <qstring.h>
#include <qwidget.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
class QTextEdit;
class QVBoxLayout;

class Editor :public QWidget
{
public:
    Editor( QWidget* parent, const char* name = 0 );
    QString text() const;
    void setText( const QString& );
protected:
    bool loadPart();

private:
    QVBoxLayout* _layout;
    QTextEdit* _edit;
    KTextEditor::Document* _doc;
    KTextEditor::View* _view;
};

#endif /* EDITOR_H */

