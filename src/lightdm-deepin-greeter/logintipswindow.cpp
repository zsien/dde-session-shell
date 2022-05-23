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
#include "public_func.h"
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
    , m_tips(findValueByQSettings<QString>(DDESESSIONCC::session_ui_configs, "Greeter", "tipsTitle", QString()))
    , m_content(findValueByQSettings<QString>(DDESESSIONCC::session_ui_configs, "Greeter", "tipsContent", QString()))
{
    initUI();

    setAttribute(Qt::WA_DeleteOnClose);
}

void LoginTipsWindow::initUI()
{
    setAccessibleName("LoginTipsWindow");

    QVBoxLayout *vLayout = new QVBoxLayout;

    // 标题
    QLabel *tipLabel = new QLabel(this);
    tipLabel->setAccessibleName("TipLabel");
    tipLabel->setAlignment(Qt::AlignHCenter);
    QPalette palette = tipLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    tipLabel->setPalette(palette);
    tipLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont font = tipLabel->font();
    font.setBold(true);
    tipLabel->setFont(font);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setTextFormat(Qt::TextFormat::PlainText);
    tipLabel->setText(m_tips);
    DFontSizeManager::instance()->bind(tipLabel, DFontSizeManager::T2, QFont::DemiBold);

    // 内容
    QLabel *contentLabel = new QLabel(this);
    contentLabel->setAccessibleName("ContentLabel");
    contentLabel->setWordWrap(true);
    QPalette contentPalette = contentLabel->palette();
    contentPalette.setColor(QPalette::WindowText, Qt::white);
    contentLabel->setPalette(contentPalette);
    contentLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setTextFormat(Qt::TextFormat::PlainText);
    contentLabel->setText(m_content);
    DFontSizeManager::instance()->bind(contentLabel, DFontSizeManager::T4);

    // 确认按钮
    QPushButton *confirmButton = new QPushButton(this);
    confirmButton->setFixedSize(90, 40);
    confirmButton->setObjectName("RequireSureButton");
    confirmButton->setText("OK");

    vLayout->addStretch();
    vLayout->addWidget(tipLabel, 0, Qt::AlignHCenter);
    vLayout->addWidget(contentLabel, 0, Qt::AlignHCenter);
    vLayout->addWidget(confirmButton, 0, Qt::AlignHCenter);
    vLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addLayout(vLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);

    connect(confirmButton, &QPushButton::clicked, this, &LoginTipsWindow::closed);
}

bool LoginTipsWindow::isValid()
{
    return (!m_tips.isEmpty() || !m_content.isEmpty());
}

