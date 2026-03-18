#include "homewindow.h"
#include <QResizeEvent>
#include <playtab.h>
#include <settingstab.h>
#include <serverstab.h>

HomeWindow::HomeWindow(QWidget* parent) : QMainWindow(parent), split(new QSplitter(this)), tabsPanel(new TabsPanel(split)) {
    setMinimumSize(960, 540);
    tabs[LauncherTab::PLAY] = new PlayTab();
    tabs[LauncherTab::SERVERS] = new ServersTab();
    tabs[LauncherTab::SETTINGS] = new SettingsTab();
    for (int i = 0; i <= static_cast<int>(LauncherTab::MAX_TAB); i++) {
        connect(tabsPanel->GetTabButton(static_cast<LauncherTab>(i)), &QPushButton::clicked, this, &HomeWindow::OnTabButtonClick);
    }
    split->setSizes({120, 840});
    split->addWidget(tabsPanel);
    split->addWidget(tabs.at(currentTab));
    setCentralWidget(split);
}

void HomeWindow::OnTabButtonClick() {
    QPushButton* pressedBtn = qobject_cast<QPushButton*>(sender());
    if (pressedBtn == nullptr) {
        return;
    }
    LauncherTab pressedTab = tabsPanel->GetLauncherTab(pressedBtn);
    SetCurrentTab(pressedTab);
}

void HomeWindow::SetCurrentTab(LauncherTab tab) {
    if (tab == currentTab) {
        return;
    }
    currentTab = tab;
    split->replaceWidget(1, tabs.at(currentTab));
}

void HomeWindow::resizeEvent(QResizeEvent* event) {
    constexpr float aspectRatio = 16.0f / 9.0f;
    int width = event->size().width();
    int height = event->size().height();

    int expectedHeight = static_cast<int>(width / aspectRatio);
    if (expectedHeight != height) {
        resize(width, expectedHeight);
    }
    QWidget::resizeEvent(event);
}

HomeWindow::~HomeWindow() {
    delete tabsPanel;
    for (auto& [k, v] : tabs) {
        delete v;
    }
    delete split;
}