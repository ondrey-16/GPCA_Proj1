//
// Created by ondrey on 4.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_SFMLCANVAS_H
#define SHOAL_FISH_ANIMATION_SFMLCANVAS_H

#include <QWidget>
#include <QTimer>
#include <SFML/Graphics.hpp>

class SFMLCanvas : public QWidget, public sf::RenderWindow {
    Q_OBJECT
    public:
    SFMLCanvas(QWidget* parent = nullptr);
protected:
    QPaintEngine* paintEngine() const override;
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    virtual void onUpdate() = 0;
private:
    QTimer* timer;
};


#endif //SHOAL_FISH_ANIMATION_SFMLCANVAS_H