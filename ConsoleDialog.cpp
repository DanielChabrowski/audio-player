#include "ConsoleDialog.hpp"

#include <QPlainTextEdit>
#include <QVBoxLayout>

ConsoleDialog::ConsoleDialog(QWidget *parent)
: QDialog{ parent }
{
    auto *layout = new QVBoxLayout(this);
    setLayout(layout);

    browser = new QPlainTextEdit(this);
    browser->setUndoRedoEnabled(false);
    browser->setReadOnly(true);
    layout->addWidget(browser);

    setSizeGripEnabled(true);
    resize(700, 300);
}

void ConsoleDialog::log(const QString &msg)
{
    browser->appendPlainText(msg);
}
