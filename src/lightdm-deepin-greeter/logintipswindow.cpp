/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     chengbo <chengbo@uniontech.com>
 *
 * Maintainer: chengbo <chengbo@uniontech.com>
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
#include "logintipswindow.h"
#include "constants.h"

#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPalette>
#include <QFile>
#include <QTextStream>

DWIDGET_USE_NAMESPACE
LoginTipsWindow::LoginTipsWindow(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void LoginTipsWindow::initUI()
{
    setAccessibleName("LoginTipsWindow");
    m_mainLayout = new QHBoxLayout(this);

    QVBoxLayout *vLayout = new QVBoxLayout(this);

    // 提示内容布局
    m_contentLabel = new QLabel();
    m_contentLabel->setAccessibleName("ContentLabel");
    m_contentLabel->setWordWrap(true);
    QPalette t_palette = m_contentLabel->palette();
    t_palette.setColor(QPalette::WindowText, Qt::white);
    m_contentLabel->setPalette(t_palette);
    m_contentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_contentLabel, DFontSizeManager::T4);
    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_contentLabel->setTextFormat(Qt::TextFormat::PlainText);

    // 获取/usr/share/dde-session-shell/dde-session-shell.conf 配置信息
    m_content = findValueByQSettings<QString>(DDESESSIONCC::session_ui_configs, "Greeter", "tipsContent", "");
    m_contentLabel->setText(m_content);

    // 提示标题布局
    m_tipLabel = new QLabel();
    m_tipLabel->setAccessibleName("TipLabel");
    m_tipLabel->setAlignment(Qt::AlignHCenter);
    QPalette palette = m_tipLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_tipLabel->setPalette(palette);
    m_tipLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont font = m_tipLabel->font();
    font.setBold(true);
    m_tipLabel->setFont(font);
    DFontSizeManager::instance()->bind(m_tipLabel, DFontSizeManager::T2, QFont::DemiBold);
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setTextFormat(Qt::TextFormat::PlainText);

    // 获取/usr/share/dde-session-shell/dde-session-shell.conf 配置信息
    m_tips = findValueByQSettings<QString>(DDESESSIONCC::session_ui_configs, "Greeter", "tipsTitle", "");
    m_tipLabel->setText(m_tips);

    // 确认按钮
    m_btn = new QPushButton();
    m_btn->setFixedSize(90, 40);
    m_btn->setObjectName("RequireSureButton");
    m_btn->setText("OK");

    vLayout->addStretch();
    vLayout->addWidget(m_tipLabel, 0, Qt::AlignHCenter);
    vLayout->addWidget(m_contentLabel, 0, Qt::AlignHCenter);
    vLayout->addWidget(m_btn, 0, Qt::AlignHCenter);
    vLayout->addStretch();

    m_mainLayout->addStretch();
    m_mainLayout->addLayout(vLayout);
    m_mainLayout->addStretch();
    setLayout(m_mainLayout);

    connect(m_btn, &QPushButton::clicked, this, [ = ] {
        // 点击确认后打开登录界面
        emit closed();
        this->close();
    });
}

bool LoginTipsWindow::isValid()
{
    return (!m_tips.isEmpty() || !m_content.isEmpty());
}

