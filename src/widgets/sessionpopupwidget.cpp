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
#include "sessionpopupwidget.h"

#include <DStyle>
#include <DStandardItem>
#include <QStandardItemModel>

const int ITEM_WIDTH = 180;
const int ITEM_HEIGHT = 34;
SessionPopupWidget::SessionPopupWidget(QWidget *parent)
    : DListView(parent)
    , m_model(new QStandardItemModel(this))
{
    initUI();
    // enter键盘响应
    connect(this, &SessionPopupWidget::activated, this, &SessionPopupWidget::itemClicked);
    connect(this, &SessionPopupWidget::clicked, this, &SessionPopupWidget::itemClicked);
}

void SessionPopupWidget::setSessionInfo(const QMap<QString, QString> &infos, const QString &curSession)
{
    m_curSession = curSession;
    QMapIterator<QString, QString> it(infos);
    while (it.hasNext()) {
        it.next();
        addItem(it.value(), it.key());
    }

    resize(ITEM_WIDTH, ITEM_HEIGHT*m_model->rowCount());
}

void SessionPopupWidget::updateCurrentSession(const QString &curSession)
{
    if (curSession == m_curSession) {
        return;
    }

    m_curSession = curSession;
    updateSelectedState();
}

void SessionPopupWidget::initUI()
{
    setAccessibleName(QStringLiteral("sessionlistview"));

    QPalette pal = palette();
    pal.setColor(DPalette::Base, QColor(235, 235, 235, int(0.05 * 255)));
    pal.setColor(QPalette::Active, QPalette::Highlight, QColor(235, 235, 235, int(0.15 * 255)));
    setPalette(pal);

    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setResizeMode(QListView::Adjust);
    setViewportMargins(0, 0, 0, 0);
    setItemSpacing(0);
    setItemSize(QSize(ITEM_WIDTH, ITEM_HEIGHT));

    QMargins itemMargins(this->itemMargins());
    itemMargins.setLeft(8);
    setItemMargins(itemMargins);

    setModel(m_model);
}

void SessionPopupWidget::addItem(const QString &icon, const QString &itemName)
{
    DStandardItem *item = new DStandardItem();
    item->setFontSize(DFontSizeManager::T6);
    item->setIcon(QIcon(icon));
    item->setText(itemName);

    QSize iconSize(12, 10);
    auto leftAction = new DViewItemAction(Qt::AlignVCenter, iconSize, iconSize, true);
    QIcon Acticon;
    if (itemName != m_curSession) {
        Acticon = QIcon();
    } else {
        Acticon = qobject_cast<DStyle *>(style())->standardIcon(DStyle::SP_MarkElement);
    }

    leftAction->setIcon(Acticon);
    item->setActionList(Qt::Edge::LeftEdge, { leftAction });

    m_model->appendRow(item);
}

void SessionPopupWidget::itemClicked(const QModelIndex &index)
{
    QString session = index.data().toString();
    if (session == m_curSession) {
        return;
    }

    m_curSession = session;
    updateSelectedState();

    Q_EMIT sessionItemClicked(session);
}

void SessionPopupWidget::updateSelectedState()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        auto item = static_cast<DStandardItem *>(m_model->item(i));
        auto action = item->actionList(Qt::Edge::LeftEdge).first();
        if (item->text() != m_curSession) {
            action->setIcon(QIcon());
            update(item->index());
            continue;
        }

        QIcon icon = qobject_cast<DStyle *>(style())->standardIcon(DStyle::SP_MarkElement);
        action->setIcon(icon);
        setCurrentIndex(item->index());
        update(item->index());
    }
}
