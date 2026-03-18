#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class ServersTab;
}
QT_END_NAMESPACE

class ServersTab : public QWidget {
    Q_OBJECT

  public:
    explicit ServersTab(QWidget* parent = nullptr);
    ~ServersTab() override;

  private:
    Ui::ServersTab* ui;
};