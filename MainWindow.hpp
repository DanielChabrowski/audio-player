#pragma once

#include <memory>

#include "ui_MainWindow.h"

struct Playlist;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    Ui::MainWindowForm ui;
    std::unique_ptr<Playlist> playlist;
};
