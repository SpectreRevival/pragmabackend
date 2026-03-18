#include "tabspanel.h"

#include "ui_TabsPanel.h"

#include <LauncherTab.h>
#include <unordered_map>

TabsPanel::TabsPanel(QWidget* parent) : QWidget(parent), ui(new Ui::TabsPanel) {
    ui->setupUi(this);
    tabButtons = {
        {LauncherTab::PLAY, ui->HomeButton},
        {LauncherTab::SERVERS, ui->ServersButton},
        {LauncherTab::SETTINGS, ui->SettingsButton}};
    for (auto& [k, v] : tabButtons) {
        tabButtonsInverse[v] = k;
    }
    this->setLayout(ui->verticalLayout);
    ui->HomeButton->setProperty("tab", QVariant::fromValue(LauncherTab::PLAY));
    ui->ServersButton->setProperty("tab", QVariant::fromValue(LauncherTab::SERVERS));
    ui->SettingsButton->setProperty("tab", QVariant::fromValue(LauncherTab::SETTINGS));
}

QPushButton* TabsPanel::GetTabButton(LauncherTab tabId) {
    auto it = tabButtons.find(tabId);
    if (it == tabButtons.end()) {
        return nullptr;
    }
    return it->second;
}

LauncherTab TabsPanel::GetLauncherTab(QPushButton* tabButton) {
    auto it = tabButtonsInverse.find(tabButton);
    if (it == tabButtonsInverse.end()) {
        qDebug() << "uh oh tried to get LauncherTab of an unknown button" << tabButton->objectName();
        return LauncherTab::PLAY;
    }
    return it->second;
}

TabsPanel::~TabsPanel() {
    delete ui;
}
