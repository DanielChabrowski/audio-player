#include "EscapableLineEdit.hpp"

#include <QKeyEvent>

void EscapableLineEdit::keyPressEvent(QKeyEvent *e)
{
    if(Qt::Key_Escape == e->key())
    {
        emit cancelEdit();
        return;
    }

    return QLineEdit::keyPressEvent(e);
}
