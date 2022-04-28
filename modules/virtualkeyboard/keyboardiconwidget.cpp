/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * Maintainer: zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "keyboardiconwidget.h"

#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

KeyboardIconWidget::KeyboardIconWidget(QWidget *parent)
    : QWidget(parent)
    , m_widgetHided(false)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void KeyboardIconWidget::setIconPath(const QString &iconPath)
{
    m_iconPath = iconPath;
    update();
}

void KeyboardIconWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect rect = this->rect();

    QPixmap pixmap(m_iconPath);
    QRect rectImage = pixmap.rect();
    rect.setX((rect.width() - rectImage.width()) /2);
    rect.setY((rect.height() - rectImage.height()) / 2);

    rect.setWidth(rectImage.width());
    rect.setHeight(rectImage.height());
    painter.drawPixmap(rect, pixmap);

    QWidget::paintEvent(event);
}

void KeyboardIconWidget::showEvent(QShowEvent *event)
{
    // 避免第一次加载后，显示触发
    if (m_widgetHided) {
        Q_EMIT widgetShowed();
        m_widgetHided = false;
    }

    QWidget::showEvent(event);
}

void KeyboardIconWidget::hideEvent(QHideEvent *event)
{
    if (this->topLevelWidget() && !topLevelWidget()->isVisible()) {
        // 通过判断顶层窗口隐藏来触发
        Q_EMIT widgetHided();
        m_widgetHided = true;
    }

    QWidget::hideEvent(event);
}

void KeyboardIconWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget *topLevelWidget = this->topLevelWidget();
        if (topLevelWidget) {
            // 获取顶层窗口，虚拟键盘窗口中需要监听顶层窗口鼠标移出事件
            Q_EMIT clicked(topLevelWidget);
        }
    }

    QWidget::mousePressEvent(event);
}
