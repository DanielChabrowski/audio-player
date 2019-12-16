#include <QLineEdit>

class EscapableLineEdit : public QLineEdit
{
public:
    using QLineEdit::QLineEdit;

    void keyPressEvent(QKeyEvent *) override;
};
