/*
* Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     fanpengcheng <fanpengcheng@uniontech.com>
*
* Maintainer: fanpengcheng <fanpengcheng@uniontech.com>
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

#include "resetpasswdwidget.h"
#include "dlineeditex.h"
#include "useravatar.h"
#include "pwqualitymanager.h"
#include "passwordlevelwidget.h"

#include <QVBoxLayout>
#include <QProcess>

#include <DPasswordEdit>
#include <DLabel>
#include <DIconButton>
#include <DDesktopServices>

DWIDGET_USE_NAMESPACE


/**
 * @brief The ResetPasswdWidget class   重置密码界面
 * @note 管理员账户密码过期时，登录时会自动跳到此界面
 */
ResetPasswdWidget::ResetPasswdWidget(std::shared_ptr<User> user, QWidget *parent)
    : QWidget(parent)
    , m_user(user)
    , m_mainLayout(new QVBoxLayout(this))
    , m_lockBtn(new DIconButton(this))
    , m_avatar(new UserAvatar(this))
    , m_nameLabel(new DLabel(m_user->displayName(), this))
    , m_tipsLabel(new DLabel(tr("Your password expired, please login gain."), this))
    , m_oldPasswdEdit(new DPasswordEdit(this))
    , m_newPasswdEdit(new DPasswordEdit(this))
    , m_levelWidget(new PasswordLevelWidget(this))
    , m_repeatPasswdEdit(new DPasswordEdit(this))
    , m_passwordHints(new DLineEditEx(this))
    , m_okBtn(new DPushButton(tr("Confirm"), this))
{
    setAccessibleName("ResetPasswdWidget");

    initUI();
    initConnections();
}

void ResetPasswdWidget::initUI()
{
    m_lockBtn->setFlat(true);
    m_lockBtn->setIcon(QIcon::fromTheme(":/misc/images/login_lock.svg"));
    m_lockBtn->setIconSize(QSize(24, 24));
    m_avatar->setFocusPolicy(Qt::NoFocus);
    m_avatar->setIcon(m_user->avatar());
    m_avatar->setAvatarSize(UserAvatar::AvatarLargeSize);

    m_nameLabel->setAlignment(Qt::AlignCenter);

    m_tipsLabel->setAlignment(Qt::AlignCenter);

    m_oldPasswdEdit->setClearButtonEnabled(false);
    m_oldPasswdEdit->setEchoMode(QLineEdit::Password);
    m_oldPasswdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_oldPasswdEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_oldPasswdEdit->setPlaceholderText(tr("Input old passoword"));

    m_newPasswdEdit->setClearButtonEnabled(false);
    m_newPasswdEdit->setEchoMode(QLineEdit::Password);
    m_newPasswdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_newPasswdEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_newPasswdEdit->setPlaceholderText(tr("Input new password"));

    m_levelWidget->reset();

    m_repeatPasswdEdit->setClearButtonEnabled(false);
    m_repeatPasswdEdit->setEchoMode(QLineEdit::Password);
    m_repeatPasswdEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_repeatPasswdEdit->lineEdit()->setAlignment(Qt::AlignCenter);
    m_repeatPasswdEdit->setPlaceholderText(tr("Input new passowrd again"));

    m_passwordHints->setClearButtonEnabled(false);
    m_passwordHints->setEchoMode(QLineEdit::Normal);
    m_passwordHints->setContextMenuPolicy(Qt::NoContextMenu);
    m_passwordHints->lineEdit()->setAlignment(Qt::AlignCenter);
    m_passwordHints->setPlaceholderText(tr("Password hints"));

    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addWidget(m_lockBtn, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_avatar, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_nameLabel, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_tipsLabel, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_oldPasswdEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_newPasswdEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_levelWidget, 0, Qt::AlignRight | Qt::AlignVCenter);
    m_mainLayout->addWidget(m_repeatPasswdEdit, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_passwordHints, 0, Qt::AlignVCenter);
    m_mainLayout->addWidget(m_okBtn, 0, Qt::AlignCenter);

    setLayout(m_mainLayout);

    // 默认焦点处于旧密码输入框中
    m_oldPasswdEdit->setFocus();
}

void ResetPasswdWidget::initConnections()
{
    connect(m_newPasswdEdit, &DLineEditEx::textChanged, this, &ResetPasswdWidget::onNewPasswordTextChanged);

    connect(m_repeatPasswdEdit, &DLineEditEx::textChanged, this, &ResetPasswdWidget::onRepeatPasswordTextEdited);
    connect(m_repeatPasswdEdit, &DLineEditEx::editingFinished, this, &ResetPasswdWidget::onRepeatPasswordEditFinished);

    connect(m_passwordHints, &DLineEdit::textEdited, this, &ResetPasswdWidget::onPasswordHintsChanged);

    connect(m_okBtn, &DPushButton::clicked, this, &ResetPasswdWidget::onOkClicked);
}

void ResetPasswdWidget::onNewPasswordTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        m_newPasswdEdit->hideAlertMessage();
        m_newPasswdEdit->setAlert(false);
        m_levelWidget->reset();
        return;
    }

    PASSWORD_LEVEL_TYPE level = PwqualityManager::instance()->newPassWdLevel(text);
    m_levelWidget->setLevel(level);

    PwqualityManager::ERROR_TYPE error = PwqualityManager::instance()->verifyPassword(m_user->displayName(), text);
    // TODO 这里的error返回异常
    if (error != PwqualityManager::ERROR_TYPE::PW_NO_ERR) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage(PwqualityManager::instance()->getErrorTips(error), m_newPasswdEdit, 2000);
    }
}

void ResetPasswdWidget::onRepeatPasswordTextEdited(const QString &text)
{
    Q_UNUSED(text)
    if (!m_repeatPasswdEdit->isAlert())
        return;

    // 清除警告信息
    m_repeatPasswdEdit->hideAlertMessage();
    m_repeatPasswdEdit->setAlert(false);
}

void ResetPasswdWidget::onRepeatPasswordEditFinished()
{
    if (sender() != m_repeatPasswdEdit)
        return;

    // 新旧密码不一致
    if (m_repeatPasswdEdit->text() != m_newPasswdEdit->text()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage(tr("Passwords do not match"), m_repeatPasswdEdit, 2000);
    }
}

void ResetPasswdWidget::onPasswordHintsChanged(const QString &text)
{
    // 限制最长14个字符
    if (text.size() > 14) {
        m_passwordHints->lineEdit()->backspace();
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Error);
        return;
    }

    // 清除警告信息
    if (m_passwordHints->isAlert()) {
        m_passwordHints->hideAlertMessage();
        m_passwordHints->setAlert(false);
    }
}

void ResetPasswdWidget::onOkClicked()
{
    if (!isInfoValid()) {
        return;
    }

    const QString &oldPassword = m_oldPasswdEdit->text();
    const QString &newPassword = m_newPasswdEdit->text();
    const QString &repeatPassword = m_repeatPasswdEdit->text();

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C");
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::MergedChannels);

    process.start("/bin/bash", QStringList() << "-c" << QString("passwd"));
    if (!m_user->isPasswordValid()) {
        process.write(QString("%1\n%2\n").arg(newPassword).arg(repeatPassword).toLatin1());
    } else {
        process.write(QString("%1\n%2\n%3").arg(oldPassword).arg(newPassword).arg(repeatPassword).toLatin1());
    }

    process.closeWriteChannel();
    process.waitForFinished();

    // exitCode = 0 表示密码修改成功
    int exitCode = process.exitCode();
    const QString& output = process.readAll();

    parseProcessResult(exitCode, output);
}

// 检查输入内容是否有效
bool ResetPasswdWidget::isInfoValid()
{
    m_oldPasswdEdit->setAlert(false);
    m_oldPasswdEdit->hideAlertMessage();
    // 旧密码为空
    if (m_oldPasswdEdit->text().isEmpty()) {
        m_oldPasswdEdit->setAlert(true);
        m_oldPasswdEdit->showAlertMessage("Password cannot be empty", m_oldPasswdEdit, 2000);
        return false;
    }

    m_newPasswdEdit->setAlert(false);
    m_newPasswdEdit->hideAlertMessage();
    // 新密码为空
    if (m_newPasswdEdit->text().isEmpty()) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage("Password cannot be empty", m_newPasswdEdit, 2000);
        return false;
    }

    m_repeatPasswdEdit->setAlert(false);
    m_repeatPasswdEdit->hideAlertMessage();
    // 重复密码为空
    if (m_repeatPasswdEdit->text().isEmpty()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage("Password cannot be empty", m_repeatPasswdEdit, 2000);
        return false;
    }

    m_passwordHints->setAlert(false);
    m_passwordHints->hideAlertMessage();
    // 密码提示信息包含密码字段
    for (auto c : m_newPasswdEdit->text()) {
        if (m_passwordHints->text().contains(c)) {
            m_passwordHints->setAlert(true);
            m_passwordHints->showAlertMessage(tr("The hint is visible to all users. Do not include the password here."), m_passwordHints, 2000);
            return false;
        }
    }

    m_newPasswdEdit->setAlert(false);
    m_newPasswdEdit->hideAlertMessage();
    // 新旧密码一致
    if (m_newPasswdEdit->text() == m_oldPasswdEdit->text()) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage(tr("New password should differ from the current one"), m_repeatPasswdEdit, 2000);
        return false;
    }

    m_newPasswdEdit->setAlert(false);
    m_newPasswdEdit->hideAlertMessage();
    // 两次输入的新密码不一致
    if (m_repeatPasswdEdit->text() != m_newPasswdEdit->text()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage(tr("Passwords do not match"), m_repeatPasswdEdit, 2000);
        return false;
    }

    return true;
}

void ResetPasswdWidget::parseProcessResult(int exitCode, const QString &output)
{
    if (exitCode == 0) {
        // TODO 修改密码成功后，设置密码提示信息
        Q_EMIT changePasswordSuccessed();
        return;
    }

    if (output.startsWith("Current Password: passwd:", Qt::CaseInsensitive)) {
        m_oldPasswdEdit->setAlert(true);
        m_oldPasswdEdit->showAlertMessage(tr("Wrong password"));
        return;
    }

    if (m_newPasswdEdit->lineEdit()->text() == m_oldPasswdEdit->lineEdit()->text()) {
        m_newPasswdEdit->setAlert(true);
        m_newPasswdEdit->showAlertMessage(tr("New password should differ from the current one"), m_oldPasswdEdit, 2000);
        return;
    }

    PwqualityManager::ERROR_TYPE error = PwqualityManager::instance()->verifyPassword(m_user->displayName(), m_newPasswdEdit->text());
    if (error == PW_NO_ERR && m_newPasswdEdit->lineEdit()->text() != m_repeatPasswdEdit->lineEdit()->text()) {
        m_repeatPasswdEdit->setAlert(true);
        m_repeatPasswdEdit->showAlertMessage(tr("Passwords do not match"), m_repeatPasswdEdit, 2000);
        return;
    }

    m_newPasswdEdit->setAlert(true);
    m_newPasswdEdit->showAlertMessage(PwqualityManager::instance()->getErrorTips(error));
}
