/*#pragma once
#include "QWidget"
#include <QPropertyAnimation>

class CarouselBackground : public QWidget {
    Q_OBJECT
public:
    CarouselBackground(QWidget *parent = nullptr);
    signals:
    void opacityChanged();
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QVector<QPixmap> images;
    int currentIndex;
    qreal opacity;
    QPropertyAnimation* fade;
};*/