#include <QLineEdit>

class EscapableLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    using QLineEdit::QLineEdit;

    void keyPressEvent(QKeyEvent *) override;

signals:
    void cancelEdit();
};
