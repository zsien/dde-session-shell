/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#ifndef USERAVATAR_H
#define USERAVATAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QPropertyAnimation>

#include <DImageButton>

class UserAvatar : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor borderSelectedColor READ borderSelectedColor WRITE setBorderSelectedColor)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)

public:
    enum AvatarSize {
        AvatarSmallSize = 80,
        AvatarNormalSize = 90,
        AvatarLargeSize = 100
    };
    Q_ENUM(AvatarSize)

    explicit UserAvatar(QWidget *parent = nullptr);
    void setIcon(const QString &iconPath);
    void setAvatarSize(const int size);
    void setDisabled(bool disable);

    QColor borderSelectedColor() const;
    void setBorderSelectedColor(const QColor &borderSelectedColor);

    QColor borderColor() const;
    void setBorderColor(const QColor &borderColor);

    int borderWidth() const;
    void setBorderWidth(int borderWidth);

    void setSelected(bool selected);

Q_SIGNALS:
    void mousePress();
    void requestDelete();
    void userAvatarClicked();
    void alphaChanged();

    void showFinished();
    void hideFinished();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QImage imageToGray(const QImage &image);

private:
    QLabel *m_iconLabel;
    QString m_iconPath;
    QColor m_borderColor;
    QColor m_borderSelectedColor;
    int m_avatarSize;
    int m_borderWidth;
    bool m_selected;
};
#endif // USERAVATAR_H
