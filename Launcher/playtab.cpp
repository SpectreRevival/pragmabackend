#include "playtab.h"

#include "ui_PlayTab.h"

PlayTab::PlayTab(QWidget* parent) : QWidget(parent), ui(new Ui::PlayTab) {
    ui->setupUi(this);
}

PlayTab::~PlayTab() {
    delete ui;
}
