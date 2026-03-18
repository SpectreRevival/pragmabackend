#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class SettingsTab;
}
QT_END_NAMESPACE

class SettingsTab : public QWidget {
    Q_OBJECT

  public:
    explicit SettingsTab(QWidget* parent = nullptr);
    ~SettingsTab() override;

  private:
    Ui::SettingsTab* ui;
};