#pragma once

#include <QLineEdit>

class EscapableLineEdit final : public QLineEdit
{
    Q_OBJECT

public:
    using QLineEdit::QLineEdit;

    void keyPressEvent(QKeyEvent *) override;

signals:
    void cancelEdit();
};
