#include "serverstab.h"

#include "ui_ServersTab.h"

ServersTab::ServersTab(QWidget* parent) : QWidget(parent), ui(new Ui::ServersTab) {
    ui->setupUi(this);
}

ServersTab::~ServersTab() {
    delete ui;
}
