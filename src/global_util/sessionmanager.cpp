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
#include "sessionmanager.h"
#include "sessionbasemodel.h"

#include <QDebug>
#include <DMessageBox>

static const QString DEFAULT_SESSION_NAME = "deepin";
static const QString WAYLAND_SESSION_NAME = "Wayland";

const QString session_standard_name(const QString &realName)
{
    const QStringList standard_icon_list = {
        "deepin",
        "fluxbox",
        "gnome",
        "plasma",
        "ubuntu",
        "xfce",
        "wayland"
    };

    for (const auto &name : standard_icon_list) {
        if (realName.contains(name, Qt::CaseInsensitive)) {
            if (realName == "deepin") {
                return "x11";
            } else {
                return name;
            }
        }
    }
    return QStringLiteral("unknown");
}

SessionManager &SessionManager::Reference()
{
    static SessionManager sessionManager;
    return sessionManager;
}

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_sessionModel(new QLightDM::SessionsModel(this))
    , m_userModel(new QLightDM::UsersModel(this))
    , m_allowSwitchingToWayland(getDConfigValue(getDefaultConfigFileName(), "allowSwitchingToWayland", false).toBool())
    , m_isWaylandExisted(false)
{
    // 判断显卡是否支持wayland
    if (isWaylandExisted() && m_allowSwitchingToWayland) {
        QDBusInterface systemDisplayInter("com.deepin.system.Display", "/com/deepin/system/Display",
                "com.deepin.system.Display", QDBusConnection::systemBus(), this);
        QDBusReply<bool> reply  = systemDisplayInter.call("SupportWayland");
        if (QDBusError::NoError == reply.error().type()) {
            m_allowSwitchingToWayland = reply.value();
        } else {
            qWarning() << "Get support wayland property failed: " << reply.error().message();
        }
    }
}

SessionManager::~SessionManager()
{

}

void SessionManager::setModel(SessionBaseModel * const model)
{
    m_model = model;
}

int SessionManager::sessionCount() const
{
    int count = m_sessionModel->rowCount(QModelIndex());
    if (m_isWaylandExisted && !m_allowSwitchingToWayland)
        --count;

    return count;
}

QString SessionManager::currentSessionKey() const
{
    int curIndex = sessionIndex(m_model->sessionKey());
    return m_sessionModel->data(m_sessionModel->index(curIndex), QLightDM::SessionsModel::KeyRole).toString();
}

QString SessionManager::currentSession() const
{
    return session_standard_name(currentSessionKey());
}

QMap<QString, QString> SessionManager::sessionInfo() const
{
    // key:sessionName; value:icon
    QMap<QString, QString> infos;
    int count = m_sessionModel->rowCount(QModelIndex());
    for (int i = 0; i < count; ++i) {
        const QString &sessionName = m_sessionModel->data(m_sessionModel->index(i), QLightDM::SessionsModel::KeyRole).toString();
        const QString &displayName = session_standard_name(sessionName);
        const QString icon = QString(":/img/sessions_icon/%1_normal.svg").arg(displayName);
        infos[displayName] = icon;
    }

    return infos;
}

void SessionManager::updateSession(const QString &userName)
{
    QString defaultSession = defaultsession();
    QString sessionName = lastLoggedInSession(userName);
    // 上次登录的是wayland，但是此次登录已经禁止使用wayland，那么使用默认的桌面
    if ((!m_allowSwitchingToWayland && !WAYLAND_SESSION_NAME.compare(sessionName, Qt::CaseInsensitive)) ||
            !WAYLAND_SESSION_NAME.compare(defaultSession, Qt::CaseInsensitive)) {
        sessionName = defaultSession;
    }

    m_model->setSessionKey(currentSessionKey());
    qDebug() << userName << "default session is: " << sessionName;
}

void SessionManager::switchSession(const QString &session)
{
    if (!WAYLAND_SESSION_NAME.compare(session, Qt::CaseInsensitive) && m_model->getSEType()) {
        // 在开启等保（高）的情况下不允许切换到wayland环境
        // TODO 合适的提示，以及合适的显示位置。
        QMessageBox::warning(nullptr, "switch wayland：",
                             "You have enabled the high system security level, thus cannot switch to the Wayland mode, "\
                             "please disable the high security level in Security Center and try again.", "sure");
        return;
    }

    int curIndex = sessionIndex(session);
    QString sessionKey = m_sessionModel->data(m_sessionModel->index(curIndex),
                                              QLightDM::SessionsModel::KeyRole).toString();
    m_model->setSessionKey(sessionKey);
}

int SessionManager::sessionIndex(const QString &sessionName) const
{
    const int count = m_sessionModel->rowCount(QModelIndex());
    Q_ASSERT(count);
    int defaultSessionIndex = 0;
    for (int i = 0; i < count; ++i) {
        const QString &sessionKey = m_sessionModel->data(m_sessionModel->index(i), QLightDM::SessionsModel::KeyRole).toString();
        if (!sessionName.compare(sessionKey, Qt::CaseInsensitive)) {
            return i;
        }

        if (!DEFAULT_SESSION_NAME.compare(sessionKey, Qt::CaseInsensitive)) {
            defaultSessionIndex = i;
        }
    }

    // NOTE: The current session does not exist
    qWarning() << "The session does not exist, using the default value.";
    return defaultSessionIndex;
}

QString SessionManager::lastLoggedInSession(const QString &userName)
{
    for (int i = 0; i < m_userModel->rowCount(QModelIndex()); ++i) {
        if (userName == m_userModel->data(m_userModel->index(i), QLightDM::UsersModel::NameRole).toString()) {
            QString session = m_userModel->data(m_userModel->index(i), QLightDM::UsersModel::SessionRole).toString();
            if (!session.isEmpty()) {
                return session;
            }
        }
    }

    return defaultsession();
}

bool SessionManager::isWaylandExisted()
{
    const int count = m_sessionModel->rowCount(QModelIndex());
    for (int i(0); i < count; ++i) {
        const QString &session_name = m_sessionModel->data(m_sessionModel->index(i), QLightDM::SessionsModel::KeyRole).toString();
        if (!WAYLAND_SESSION_NAME.compare(session_name, Qt::CaseInsensitive)) {
            m_isWaylandExisted = true;
        }
    }

    return false;
}

QString SessionManager::defaultsession() const
{
    return getDConfigValue(getDefaultConfigFileName(),"defaultSession", DEFAULT_SESSION_NAME).toString();
}
