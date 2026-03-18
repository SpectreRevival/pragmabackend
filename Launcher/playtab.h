#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class PlayTab;
}
QT_END_NAMESPACE

class PlayTab : public QWidget {
    Q_OBJECT

  public:
    explicit PlayTab(QWidget* parent = nullptr);
    ~PlayTab() override;

  private:
    Ui::PlayTab* ui;
};
