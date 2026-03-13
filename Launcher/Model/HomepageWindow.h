#pragma once
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class HomepageWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit HomepageWindow(QWidget* parent = nullptr);
    ~HomepageWindow();

    private:
    Ui::MainWindow* ui;
};