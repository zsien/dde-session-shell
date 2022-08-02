/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
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

#include "controlwidget.h"
#include "sessionbasemodel.h"
#include "kblayoutlistview.h"

#include "modules_loader.h"
#include "tray_module_interface.h"
#include "tipswidget.h"
#include "public_func.h"
#include "tipcontentwidget.h"
#include "sessionpopupwidget.h"
#include "sessionmanager.h"
#include "userlistpopupwidget.h"

#include <DFloatingButton>
#include <DArrowRectangle>
#include <DPushButton>
#include <DConfig>

#include <QEvent>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QWheelEvent>
#include <QMenu>

#define BUTTON_ICON_SIZE QSize(26,26)
#define BUTTON_SIZE QSize(52,52)

using namespace dss;
DCORE_USE_NAMESPACE

bool FlotingButton::eventFilter(QObject *watch, QEvent *event)
{
    if (watch == this) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            if (e->button() == Qt::RightButton) {
                Q_EMIT requestShowMenu();
            }
        } else if (event->type() == QEvent::Enter) {
            Q_EMIT requestShowTips();
        } else if (event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonPress) {
            Q_EMIT requestHideTips();
        }
    }

    return false;
}

ControlWidget::ControlWidget(const SessionBaseModel *model, QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_switchUserBtn(new FlotingButton(this))
    , m_powerBtn(new FlotingButton(this))
    , m_sessionBtn(new FlotingButton(this))
    , m_keyboardBtn(nullptr)
    , m_contextMenu(new QMenu(this))
    , m_tipContentWidget(nullptr)
    , m_tipsWidget(new TipsWidget(parent ? parent->window() : nullptr))
    , m_arrowRectWidget(new DArrowRectangle(DArrowRectangle::ArrowBottom, this))
    , m_kbLayoutListView(nullptr)
    , m_sessionPopupWidget(nullptr)
    , m_userListPopupWidget(nullptr)
{
    setModel(model);
    initUI();
    initConnect();
}

void ControlWidget::setModel(const SessionBaseModel *model)
{
    m_model = model;
    setUser(model->currentUser());
}

void ControlWidget::setUser(std::shared_ptr<User> user)
{
    for (const QMetaObject::Connection &connection : qAsConst(m_connectionList))
        user.get()->disconnect(connection);

    m_connectionList.clear();

    m_curUser = user;

    m_connectionList << connect(m_curUser.get(), &User::keyboardLayoutChanged, this, &ControlWidget::setKeyboardType)
                     << connect(m_curUser.get(), &User::keyboardLayoutListChanged, this, &ControlWidget::setKeyboardList);
}

void ControlWidget::initKeyboardLayoutList()
{
    /* 键盘布局选择菜单 */
    const QString language = m_curUser->keyboardLayout();
    m_kbLayoutListView = new KBLayoutListView(language, this);
    m_kbLayoutListView->setAccessibleName(QStringLiteral("KbLayoutlistview"));
    m_kbLayoutListView->initData(m_curUser->keyboardLayoutList());
    m_kbLayoutListView->setMinimumWidth(DDESESSIONCC::KEYBOARDLAYOUT_WIDTH);
    m_kbLayoutListView->setMaximumSize(DDESESSIONCC::KEYBOARDLAYOUT_WIDTH, DDESESSIONCC::LAYOUTBUTTON_HEIGHT * 7);
    m_kbLayoutListView->setFocusPolicy(Qt::NoFocus);

    const QStringList languageList = language.split(";");
    if (!languageList.isEmpty())
        static_cast<QAbstractButton *>(m_keyboardBtn)->setText(languageList.at(0));

    // 无特效模式时，让窗口圆角
    m_arrowRectWidget->setProperty("_d_radius_force", true);
    m_arrowRectWidget->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint);
    m_arrowRectWidget->setMargin(0);
    m_arrowRectWidget->setShadowBlurRadius(20);
    m_arrowRectWidget->setRadius(6);
    m_arrowRectWidget->setShadowYOffset(2);
    m_arrowRectWidget->setShadowXOffset(0);
    m_arrowRectWidget->setArrowWidth(18);
    m_arrowRectWidget->setArrowHeight(10);
    m_arrowRectWidget->setMinimumWidth(DDESESSIONCC::KEYBOARDLAYOUT_WIDTH);
    m_arrowRectWidget->setMaximumSize(DDESESSIONCC::KEYBOARDLAYOUT_WIDTH, DDESESSIONCC::LAYOUTBUTTON_HEIGHT * 7);
    m_arrowRectWidget->setFocusPolicy(Qt::NoFocus);

    QPalette pal = m_arrowRectWidget->palette();
    pal.setColor(DPalette::Inactive, DPalette::Base, QColor(235, 235, 235, int(0.05 * 255)));
    setPalette(pal);

    m_kbLayoutListView->setVisible(false);
    connect(m_kbLayoutListView, &KBLayoutListView::itemClicked, this, &ControlWidget::onItemClicked);
    connect(m_kbLayoutListView, &KBLayoutListView::sizeChange, this, &ControlWidget::resizeArrowWidget);
}

void ControlWidget::initUI()
{
    m_tipsWidget->setVisible(false);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 60, 0);
    m_mainLayout->setSpacing(26);
    m_mainLayout->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    m_sessionBtn->setIconSize(BUTTON_ICON_SIZE);
    m_sessionBtn->setFixedSize(BUTTON_SIZE);
    m_sessionBtn->setAutoExclusive(true);
    m_sessionBtn->setBackgroundRole(DPalette::Button);
    m_sessionBtn->hide();
    m_sessionBtn->setTipText(tr("session"));

    m_keyboardBtn = new FlotingButton(this);
    m_keyboardBtn->setAccessibleName("KeyboardLayoutBtn");
    m_keyboardBtn->setFixedSize(BUTTON_SIZE);
    m_keyboardBtn->setAutoExclusive(true);
    m_keyboardBtn->setBackgroundRole(DPalette::Button);
    m_keyboardBtn->setAutoExclusive(true);
    static_cast<QAbstractButton *>(m_keyboardBtn)->setText(QString());
    m_keyboardBtn->setTipText(tr("keyboard layout"));

    // 给显示文字的按钮设置样式
    QPalette pal = m_keyboardBtn->palette();
    pal.setColor(QPalette::Window, QColor(Qt::red));
    pal.setColor(QPalette::HighlightedText, QColor(Qt::white));
    m_keyboardBtn->setPalette(pal);

    m_switchUserBtn->setAccessibleName("SwitchUserBtn");
    m_switchUserBtn->setIcon(QIcon::fromTheme(":/img/bottom_actions/userswitch_normal.svg"));
    m_switchUserBtn->setIconSize(BUTTON_ICON_SIZE);
    m_switchUserBtn->setFixedSize(BUTTON_SIZE);
    m_switchUserBtn->setAutoExclusive(true);
    m_switchUserBtn->setBackgroundRole(DPalette::Button);
    m_switchUserBtn->setTipText(tr("switch user"));

    m_powerBtn->setAccessibleName("PowerBtn");
    m_powerBtn->setIcon(QIcon(":/img/bottom_actions/shutdown_normal.svg"));
    m_powerBtn->setIconSize(BUTTON_ICON_SIZE);
    m_powerBtn->setFixedSize(BUTTON_SIZE);
    m_powerBtn->setAutoExclusive(true);
    m_powerBtn->setBackgroundRole(DPalette::Button);
    m_powerBtn->setTipText(tr("shutdown"));

    if (m_curUser->keyboardLayoutList().size() < 2)
        m_keyboardBtn->hide();

    m_mainLayout->addWidget(m_sessionBtn);
    m_mainLayout->addWidget(m_keyboardBtn);
    m_mainLayout->addWidget(m_switchUserBtn);
    m_mainLayout->addWidget(m_powerBtn);

    QHash<QString, module::BaseModuleInterface *> modules = module::ModulesLoader::instance().moduleList();
    for (module::BaseModuleInterface *module : modules.values()) {
        addModule(module);
    }

    updateLayout();

    // 初始化键盘布局列表
    initKeyboardLayoutList();
}

void ControlWidget::initConnect()
{
    connect(&module::ModulesLoader::instance(), &module::ModulesLoader::moduleFound, this, &ControlWidget::addModule);

    connect(m_sessionBtn, &FlotingButton::clicked, this, &ControlWidget::showSessionPopup);
    connect(m_sessionBtn, &FlotingButton::requestShowTips, this, &ControlWidget::showInfoTips);
    connect(m_sessionBtn, &FlotingButton::requestHideTips, this, &ControlWidget::hideInfoTips);

    connect(m_switchUserBtn, &FlotingButton::clicked, this, &ControlWidget::showUserListPopupWidget);
    connect(m_switchUserBtn, &FlotingButton::requestShowTips, this, &ControlWidget::showInfoTips);
    connect(m_switchUserBtn, &FlotingButton::requestHideTips, this, &ControlWidget::hideInfoTips);

    connect(m_powerBtn, &FlotingButton::clicked, this, &ControlWidget::requestShutdown);
    connect(m_powerBtn, &FlotingButton::requestShowTips, this, &ControlWidget::showInfoTips);
    connect(m_powerBtn, &FlotingButton::requestHideTips, this, &ControlWidget::hideInfoTips);

    connect(m_keyboardBtn, &FlotingButton::clicked, this, &ControlWidget::setKBLayoutVisible);
    connect(m_keyboardBtn, &FlotingButton::requestShowTips, this, &ControlWidget::showInfoTips);
    connect(m_keyboardBtn, &FlotingButton::requestHideTips, this, &ControlWidget::hideInfoTips);

    connect(m_model, &SessionBaseModel::currentUserChanged, this, &ControlWidget::setUser);
}

void ControlWidget::addModule(module::BaseModuleInterface *module)
{
    if (module && module->type() != module::BaseModuleInterface::TrayType)
        return;

    module::TrayModuleInterface *trayModule = dynamic_cast<module::TrayModuleInterface *>(module);
    if (!trayModule)
        return;

    trayModule->init();

    FlotingButton *button = new FlotingButton(this);
    button->setIconSize(QSize(26, 26));
    button->setFixedSize(QSize(52, 52));
    button->setAutoExclusive(true);
    button->setBackgroundRole(DPalette::Button);

    if (QWidget *trayWidget = trayModule->itemWidget()) {
        trayWidget->setParent(this);
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);
        layout->addWidget(trayWidget);

        button->setLayout(layout);
    } else {
        button->setIcon(QIcon(trayModule->icon()));
    }

    m_modules.insert(trayModule->key(), button);

    connect(button, &FlotingButton::requestShowMenu, this, [ = ] {
        const QString menuJson = trayModule->itemContextMenu();
        if (menuJson.isEmpty())
            return;

        QJsonDocument jsonDocument = QJsonDocument::fromJson(menuJson.toLocal8Bit().data());
        if (jsonDocument.isNull())
            return;

        QJsonObject jsonMenu = jsonDocument.object();

        m_contextMenu->clear();
        QJsonArray jsonMenuItems = jsonMenu.value("items").toArray();
        for (auto item : jsonMenuItems) {
            QJsonObject itemObj = item.toObject();
            QAction *action = new QAction(itemObj.value("itemText").toString());
            action->setCheckable(itemObj.value("isCheckable").toBool());
            action->setChecked(itemObj.value("checked").toBool());
            action->setData(itemObj.value("itemId").toString());
            action->setEnabled(itemObj.value("isActive").toBool());
            m_contextMenu->addAction(action);
        }

        QAction *action = m_contextMenu->exec(QCursor::pos());
        if (action)
            trayModule->invokedMenuItem(action->data().toString(), true);
    });

    connect(button, &FlotingButton::requestShowTips, this, [ = ] {
        if (trayModule->itemTipsWidget()) {
            m_tipsWidget->setContent(trayModule->itemTipsWidget());
            QPoint p = m_tipsWidget->parentWidget() ? m_tipsWidget->parentWidget()->mapFromGlobal(mapToGlobal(button->pos())) : mapToGlobal(button->pos());
            m_tipsWidget->show(p.x() + button->width() / 2, p.y());
        }
    });

    connect(button, &FlotingButton::requestHideTips, this, [ = ] {
        if (m_tipsWidget->getContent())
            m_tipsWidget->getContent()->setVisible(false);
        m_tipsWidget->hide();
    });

    connect(button, &DFloatingButton::clicked, this, [this, trayModule] {
        emit requestShowModule(trayModule->key());
    }, Qt::UniqueConnection);


    updateLayout();
}

void ControlWidget::updateLayout()
{
    QObjectList moduleWidgetList = m_mainLayout->children();
    for (QWidget *moduleWidget : m_modules.values()) {
        if (moduleWidgetList.contains(moduleWidget)) {
            moduleWidgetList.removeAll(moduleWidget);
        }
        m_mainLayout->insertWidget(1, moduleWidget);
    }
    for (QObject *moduleWidget : moduleWidgetList) {
        m_mainLayout->removeWidget(qobject_cast<QWidget *>(moduleWidget));
    }
}

void ControlWidget::setUserSwitchEnable(const bool visible)
{
    m_switchUserBtn->setVisible(visible);

    updateTapOrder();
}

void ControlWidget::setSessionSwitchEnable(const bool visible)
{
    if (!visible)  {
        m_sessionBtn->hide();
        return;
    }

    m_sessionBtn->show();

#ifdef SHENWEI_PLATFORM
    m_sessionBtn->setProperty("normalIcon", ":/img/sessions/unknown_indicator_normal.svg");
    m_sessionBtn->setProperty("hoverIcon", ":/img/sessions/unknown_indicator_hover.svg");
    m_sessionBtn->setProperty("checkedIcon", ":/img/sessions/unknown_indicator_press.svg");
#endif

    updateTapOrder();
}

void ControlWidget::chooseToSession(const QString &session)
{
    if (!m_sessionBtn || session.isEmpty()) {
        return;
    }

    const QString sessionId = session.toLower();
    QString normalIcon = QString(":/img/sessions/%1_indicator_normal.svg").arg(sessionId);
    if (sessionId == "deepin") {
        normalIcon = QString(":/img/sessions/%1_indicator_normal.svg").arg("x11");
    }

    if (QFile(normalIcon).exists()) {
#ifndef SHENWEI_PLATFORM
        m_sessionBtn->setIcon(QIcon::fromTheme(normalIcon));
#else
        m_sessionBtn->setProperty("normalIcon", normalIcon);
        m_sessionBtn->setProperty("hoverIcon", hoverIcon);
        m_sessionBtn->setProperty("checkedIcon", checkedIcon);
#endif
    } else {
#ifndef SHENWEI_PLATFORM
        m_sessionBtn->setIcon(QIcon::fromTheme(":/img/sessions/unknown_indicator_normal.svg"));
#else
        m_sessionBtn->setProperty("normalIcon", ":/img/sessions/unknown_indicator_normal.svg");
        m_sessionBtn->setProperty("hoverIcon", ":/img/sessions/unknown_indicator_hover.svg");
        m_sessionBtn->setProperty("checkedIcon", ":/img/sessions/unknown_indicator_press.svg");
#endif
    }
}

void ControlWidget::setKBLayoutVisible()
{
    DFloatingButton *layoutButton = static_cast<DFloatingButton *>(sender());
    if (!layoutButton)
        return;

    QWidget *content = m_arrowRectWidget->getContent();
    if (!content) {
        m_arrowRectWidget->setContent(m_kbLayoutListView);
    } else if (content != m_kbLayoutListView) {
        content->setVisible(false);
        m_arrowRectWidget->setContent(m_kbLayoutListView);
    }

    // 算上三角形的高度
    QPoint pos = mapToGlobal(layoutButton->pos()) - QPoint((m_kbLayoutListView->width() - layoutButton->width())/2, m_kbLayoutListView->height() + 10);
    m_arrowRectWidget->setGeometry(QRect(pos, m_kbLayoutListView->size() + QSize(0, 10)));
    m_arrowRectWidget->setVisible(!m_arrowRectWidget->isVisible());
}

void ControlWidget::setKeyboardType(const QString &str)
{
    // 初始化当前键盘布局输入法类型
    QString currentText = str.split(";").first();
    /* special mark in Japanese */
    if (currentText.contains("/"))
        currentText = currentText.split("/").last();

    static_cast<QAbstractButton *>(m_keyboardBtn)->setText(currentText);

    // 更新键盘布局列表
    m_kbLayoutListView->updateList(str);
}

void ControlWidget::setKeyboardList(const QStringList &str)
{
    if (str.size() < 2)
        m_keyboardBtn->hide();
    else
        m_keyboardBtn->show();

    // 初始化当前键盘布局列表
    if (m_kbLayoutListView)
        m_kbLayoutListView->initData(str);
}

void ControlWidget::onItemClicked(const QString &str)
{
    // 初始化当前键盘布局输入法类型
    QString currentText = str.split(";").first();
    /* special mark in Japanese */
    if (currentText.contains("/"))
        currentText = currentText.split("/").last();

    static_cast<QAbstractButton *>(m_keyboardBtn)->setText(currentText);
    m_arrowRectWidget->hide();
    m_curUser->setKeyboardLayout(str);
}

void ControlWidget::resizeArrowWidget()
{
    if (m_arrowRectWidget)
        m_arrowRectWidget->setFixedSize(m_kbLayoutListView->size() + QSize(0, 10));
}

void ControlWidget::showSessionPopup()
{
    if (!m_sessionPopupWidget) {
        m_sessionPopupWidget = new SessionPopupWidget(this);
        connect(m_sessionPopupWidget, &SessionPopupWidget::sessionItemClicked, [this](QString session) {
            m_arrowRectWidget->hide();
            Q_EMIT requestSwitchSession(session);
        });

        m_sessionPopupWidget->setSessionInfo(SessionManager::Reference().sessionInfo(),
                                             SessionManager::Reference().currentSession());
    } else {
        m_sessionPopupWidget->updateCurrentSession(SessionManager::Reference().currentSession());
    }

    QWidget *content = m_arrowRectWidget->getContent();
    if (!content) {
        m_arrowRectWidget->setContent(m_sessionPopupWidget);
    } else if (content != m_sessionPopupWidget) {
        content->setVisible(false);
        m_arrowRectWidget->setContent(m_sessionPopupWidget);
    }

    // 算上三角形的高度
    QPoint pos = mapToGlobal(m_sessionBtn->pos()) - QPoint((m_sessionPopupWidget->width() - m_sessionBtn->width())/2,
                                                           m_sessionPopupWidget->height() + 10);
    m_arrowRectWidget->setGeometry(QRect(pos, m_sessionPopupWidget->size() + QSize(0, 10)));
    m_arrowRectWidget->setVisible(!m_arrowRectWidget->isVisible());
}

void ControlWidget::showUserListPopupWidget()
{
    if (!m_userListPopupWidget) {
        m_userListPopupWidget = new UserListPopupWidget(m_model, this);
        connect(m_userListPopupWidget, &UserListPopupWidget::requestSwitchToUser, [ this ](std::shared_ptr<User> user) {
            m_arrowRectWidget->hide();
            Q_EMIT requestSwitchUser(user);
        });
    }

    QWidget *content = m_arrowRectWidget->getContent();
    if (!content) {
        m_arrowRectWidget->setContent(m_userListPopupWidget);
    } else if (content != m_userListPopupWidget) {
        content->setVisible(false);
        m_arrowRectWidget->setContent(m_userListPopupWidget);
    }

    // 算上三角形的高度
    QPoint pos = mapToGlobal(m_switchUserBtn->pos()) - QPoint((m_userListPopupWidget->width() - m_switchUserBtn->width()) / 2,
                                                              m_userListPopupWidget->height() + 10);
    m_arrowRectWidget->setGeometry(QRect(pos, m_userListPopupWidget->size() + QSize(0, 10)));
    m_arrowRectWidget->setVisible(!m_arrowRectWidget->isVisible());
}

void ControlWidget::keyReleaseEvent(QKeyEvent *event)
{
    // 处理键盘上左右键焦点切换；本身支持左右焦点切换，但最左和最右切换的时候不能循环焦点，需单独处理
    static int currentIndex = -1;
    int key = event->key();
    switch (key) {
        case Qt::Key_Left:
            if (currentIndex == 0 && m_showedBtnList.last()) {
                m_showedBtnList.last()->setFocus();
            }
            break;
        case Qt::Key_Right:
            if ((currentIndex + 1) == m_showedBtnList.count() && m_showedBtnList.first()) {
                m_showedBtnList.first()->setFocus();
            }
            break;
        default:
            break;
    }

    if (key == Qt::Key_Tab || key == Qt::Key_Left || key == Qt::Key_Right) {
        currentIndex = focusedBtnIndex();
    }

    QWidget::keyReleaseEvent(event);
}

void ControlWidget::updateTapOrder()
{
    // 找出所有显示的按钮
    m_showedBtnList.clear();
    for(int i = 0; i < m_mainLayout->count(); ++i) {
        FlotingButton *btn = qobject_cast<FlotingButton *>(m_mainLayout->itemAt(i)->widget());
        if (btn && btn->isVisible()) {
            m_showedBtnList.append(btn);
        }
    }

    // 按从左到右的顺序设置tab order
    for (int i = 0; i < m_showedBtnList.size(); ++i) {
        if ((i + 1) < m_showedBtnList.size()) {
            setTabOrder(m_showedBtnList[i], m_showedBtnList[i + 1]);
        }
    }
}

int ControlWidget::focusedBtnIndex()
{
    for (int index = 0; index < m_showedBtnList.count(); index++) {
        FlotingButton *btn = m_showedBtnList.at(index);
        if (btn && btn->hasFocus()) {
            return index;
        }
    }

    return -1;
}

void ControlWidget::showInfoTips()
{
    FlotingButton * button = qobject_cast<FlotingButton *>(sender());
    if (!button) {
        return;
    }

    if (!m_tipContentWidget) {
        m_tipContentWidget = new TipContentWidget(this);
    }

    m_tipContentWidget->setText(button->tipText());
    m_tipsWidget->setContent(m_tipContentWidget);

    m_tipsWidget->show(mapToGlobal(button->pos()).x() + button->width() / 2, mapToGlobal(button->pos()).y());
}

void ControlWidget::hideInfoTips()
{
    if (m_tipsWidget->getContent()) {
        m_tipsWidget->getContent()->setVisible(false);
    }

    m_tipsWidget->hide();
}

void ControlWidget::showEvent(QShowEvent *event)
{
    updateTapOrder();

    QWidget::showEvent(event);
}