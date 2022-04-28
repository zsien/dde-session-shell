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

#ifndef KEYBOARDICONWIDGET_H
#define KEYBOARDICONWIDGET_H

#include <QWidget>

class KeyboardIconWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KeyboardIconWidget(QWidget *parent = nullptr);

signals:
    void widgetHided();
    void widgetShowed();
    void clicked(QWidget *parent);

public:
    void setIconPath(const QString &iconPath);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

private:
    QString m_iconPath;
    bool m_widgetHided;
};

#endif // KEYBOARDICONWIDGET_H
