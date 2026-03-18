#pragma once
#include "QMainWindow"

#include <QSplitter>
#include <QWidget>
#include <tabspanel.h>

QT_BEGIN_NAMESPACE
namespace Ui {
    class HomeWindow;
}
QT_END_NAMESPACE

class HomeWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit HomeWindow(QWidget* parent = nullptr);
    ~HomeWindow() override;

    void resizeEvent(QResizeEvent* event) override;

    void SetCurrentTab(LauncherTab newTab);

  private:
    QSplitter* split;
    TabsPanel* tabsPanel;
    std::unordered_map<LauncherTab, QWidget*> tabs;
    LauncherTab currentTab = LauncherTab::PLAY;
    void OnTabButtonClick();
};