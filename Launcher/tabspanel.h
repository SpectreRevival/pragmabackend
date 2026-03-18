#pragma once
#include "LauncherTab.h"

#include <QPushButton>
#include <QWidget>
#include <unordered_map>

QT_BEGIN_NAMESPACE
namespace Ui {
    class TabsPanel;
}
QT_END_NAMESPACE

class TabsPanel : public QWidget {
    Q_OBJECT

  public:
    explicit TabsPanel(QWidget* parent = nullptr);
    ~TabsPanel() override;
    QPushButton* GetTabButton(LauncherTab tabId);
    LauncherTab GetLauncherTab(QPushButton* tabButton);



  private:
    Ui::TabsPanel* ui;
    std::unordered_map<LauncherTab, QPushButton*> tabButtons;
    std::unordered_map<QPushButton*, LauncherTab> tabButtonsInverse;
};