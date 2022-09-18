#pragma once

#include <QDialog>

class QPlainTextEdit;

class ConsoleDialog final : public QDialog
{
    Q_OBJECT

public:
    ConsoleDialog(QWidget *parent = nullptr);

    void log(const QString &msg);

private:
    QPlainTextEdit *browser;
};
