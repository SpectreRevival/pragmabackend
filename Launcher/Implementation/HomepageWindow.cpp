#include <HomepageWindow.h>
#include <ui_mainwindow.h>

HomepageWindow::HomepageWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

HomepageWindow::~HomepageWindow() {
    delete ui;
}