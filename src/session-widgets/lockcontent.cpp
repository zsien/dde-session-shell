#include "lockcontent.h"

#include "src/widgets/controlwidget.h"
#include "sessionbasemodel.h"
#include "userframe.h"
#include "src/widgets/shutdownwidget.h"
#include "src/widgets/virtualkbinstance.h"
#include "src/widgets/logowidget.h"
#include "src/widgets/timewidget.h"
#include "userlogininfo.h"
#include "userloginwidget.h"
#include "userframelist.h"

#include <QMouseEvent>

LockContent::LockContent(SessionBaseModel *const model, QWidget *parent, bool islockapp)
    : SessionBaseWindow(parent)
    , m_model(model)
    , m_imageBlurInter(new ImageBlur("com.deepin.daemon.Accounts", "/com/deepin/daemon/ImageBlur", QDBusConnection::systemBus(), this))
    , m_virtualKB(nullptr)
    , m_translator(new QTranslator)
    , m_userLoginInfo(new UserLoginInfo(model))
{
    m_controlWidget = new ControlWidget;
    m_userFrame = new UserFrame;
    m_shutdownFrame = new ShutdownWidget;
    m_logoWidget = new LogoWidget;

    uint userID = m_model->currentUser()->uid();
    const QString lockDBusPath = QString("/com/deepin/daemon/Accounts/User%1").arg(userID);
    m_24HourFormatInter = new QDBusInterface("com.deepin.daemon.Accounts", lockDBusPath, "com.deepin.daemon.Accounts.User",
                                             QDBusConnection::systemBus(), this);

    m_timeWidget = new TimeWidget();
    m_mediaWidget = nullptr;

    m_shutdownFrame->setModel(model);
    m_userFrame->setModel(model);

    model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    setCenterTopWidget(m_timeWidget);
    setLeftBottomWidget(m_logoWidget);
    connect(model, &SessionBaseModel::currentUserChanged, this, [ = ](std::shared_ptr<User> user) {
        if (user.get()) {
            m_logoWidget->updateLocale(user->locale().split(".").first());
        }
    });

    switch (model->currentType()) {
    case SessionBaseModel::AuthType::LockType:
        setMPRISEnable(true);
        break;
    default:
        break;
    }
    setRightBottomWidget(m_controlWidget);

    // init connect
    connect(model, &SessionBaseModel::currentUserChanged, this, &LockContent::onCurrentUserChanged);
    connect(m_userFrame, &UserFrame::requestSwitchUser, this, &LockContent::requestSwitchToUser);
    connect(m_userFrame, &UserFrame::requestSwitchUser, this, &LockContent::restoreMode);
    connect(m_controlWidget, &ControlWidget::requestSwitchUser, this, [ = ] {
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::UserMode) return;
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
    });
    connect(m_controlWidget, &ControlWidget::requestShutdown, this, [ = ] {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PowerMode);
    });
    connect(m_controlWidget, &ControlWidget::requestSwitchVirtualKB, this, &LockContent::toggleVirtualKB);
    connect(m_userFrame, &UserFrame::hideFrame, this, &LockContent::restoreMode);

    //lixin
    connect(m_userLoginInfo, &UserLoginInfo::requestAuthUser, this, &LockContent::requestAuthUser);
    connect(m_userLoginInfo, &UserLoginInfo::hideUserFrameList, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestSwitchUser, this, &LockContent::requestSwitchToUser);
    connect(m_userLoginInfo, &UserLoginInfo::requestSwitchUser, this, &LockContent::restoreMode);
    connect(m_userLoginInfo, &UserLoginInfo::requestSetLayout, this, &LockContent::requestSetLayout);
    connect(m_shutdownFrame, &ShutdownWidget::abortOperation, this, &LockContent::restoreMode);
    if (islockapp) {
        // 锁屏，点击关机，需要提示“输入密码以关机”。登录不需要这个提示
        connect(m_shutdownFrame, &ShutdownWidget::abortOperation, m_userLoginInfo, [ = ] {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        });
    }
    connect(model, &SessionBaseModel::onStatusChanged, this, &LockContent::onStatusChanged);

    auto initVirtualKB = [&](bool hasvirtualkb) {
        if (hasvirtualkb && !m_virtualKB) {
            connect(&VirtualKBInstance::Instance(), &VirtualKBInstance::initFinished, this, [&] {
                m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
                m_controlWidget->setVirtualKBVisible(true);
            }, Qt::QueuedConnection);
            VirtualKBInstance::Instance().init();
        }
    };

    connect(model, &SessionBaseModel::hasVirtualKBChanged, this, initVirtualKB);
    connect(model, &SessionBaseModel::onUserListChanged, this, &LockContent::onUserListChanged);
    connect(m_imageBlurInter, &ImageBlur::BlurDone, this, &LockContent::onBlurDone);

    QTimer::singleShot(0, this, [ = ] {
        onCurrentUserChanged(model->currentUser());
        initVirtualKB(model->hasVirtualKB());
        onUserListChanged(model->userList());
    });
}

void LockContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    if (user.get() == nullptr) return; // if dbus is async

    // set user language
    qApp->removeTranslator(m_translator);
    const QString locale { user->locale().split(".").first() };
    m_translator->load("/usr/share/dde-session-shell/translations/dde-session-shell_" + QLocale(locale.isEmpty() ? "en_US" : locale).name());
    qApp->installTranslator(m_translator);

    m_timeWidget->updateLocale(user->locale());

    for (auto connect : m_currentUserConnects) {
        m_user.get()->disconnect(connect);
    }

    m_currentUserConnects.clear();

    m_user = user;

    m_currentUserConnects << connect(user.get(), &User::greeterBackgroundPathChanged, this, &LockContent::updateBackground, Qt::UniqueConnection);

    //lixin
    m_userLoginInfo->setUser(user);

    //TODO: refresh blur image
    QTimer::singleShot(0, this, [ = ] {
        updateBackground(user->greeterBackgroundPath());
    });
}

void LockContent::pushUserFrame()
{
//    setCenterContent(m_userFrame);
//    m_userFrame->expansion(true);

    setCenterContent(m_userLoginInfo->getUserFrameList());
}

void LockContent::pushShutdownFrame()
{
    setCenterContent(m_shutdownFrame);
}

void LockContent::setMPRISEnable(const bool state)
{
    if (m_mediaWidget) {
        m_mediaWidget->setVisible(state);
    } else {
        m_mediaWidget = new MediaWidget;
        m_mediaWidget->initMediaPlayer();
        setCenterBottomWidget(m_mediaWidget);
    }
}

void LockContent::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    switch (status) {
    case SessionBaseModel::ModeStatus::PasswordMode:
        restoreCenterContent();
        break;
    case SessionBaseModel::ModeStatus::ConfirmPasswordMode:
        restoreCenterContent();
        m_model->setAbortConfirm(true);
        break;
    case SessionBaseModel::ModeStatus::UserMode:
        pushUserFrame();
        break;
    case SessionBaseModel::ModeStatus::PowerMode:
        pushShutdownFrame();
        break;
    default:
        break;
    }
}

void LockContent::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ConfirmPasswordMode)
        m_model->setAbortConfirm(false);

    // hide keyboardlayout widget
    m_userLoginInfo->hideKBLayout();
    restoreCenterContent();

    return SessionBaseWindow::mouseReleaseEvent(event);
}

void LockContent::showEvent(QShowEvent *event)
{
    onStatusChanged(m_model->currentModeState());

    m_timeWidget->set24HourFormat(m_24HourFormatInter->property("Use24HourFormat").toBool());

    tryGrabKeyboard();

    return QFrame::showEvent(event);
}

void LockContent::hideEvent(QHideEvent *event)
{
    return QFrame::hideEvent(event);
}

void LockContent::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, this, [ = ] {
        if (m_virtualKB && m_virtualKB->isVisible())
        {
            updateVirtualKBPosition();
        }
    });

    return QFrame::resizeEvent(event);
}

void LockContent::restoreCenterContent()
{
    setCenterContent(m_userLoginInfo->getUserLoginWidget());
}

void LockContent::restoreMode()
{
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
}

void LockContent::updateBackground(const QString &path)
{
//    const QString &wallpaper = m_imageBlurInter->Get(path);

//    emit requestBackground(wallpaper.isEmpty() ? path : wallpaper);
    emit requestBackground(path);
}

void LockContent::onBlurDone(const QString &source, const QString &blur, bool status)
{
    const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

    if (status && m_model->currentUser()->greeterBackgroundPath() == sourcePath)
        emit requestBackground(blur);
}

void LockContent::toggleVirtualKB()
{
    if (!m_virtualKB) {
        VirtualKBInstance::Instance();
        QTimer::singleShot(500, this, [ = ] {
            m_virtualKB = VirtualKBInstance::Instance().virtualKBWidget();
            qDebug() << "init virtualKB over." << m_virtualKB;
            toggleVirtualKB();
        });
        return;
    }

    m_virtualKB->setParent(this);
    m_virtualKB->setVisible(!m_virtualKB->isVisible());
    m_virtualKB->raise();
    m_userLoginInfo->getUserLoginWidget()->setPassWordEditFocus();

    updateVirtualKBPosition();
}

void LockContent::updateVirtualKBPosition()
{
    const QPoint point = mapToParent(QPoint((width() - m_virtualKB->width()) / 2, height() - m_virtualKB->height() - 50));
    m_virtualKB->move(point);
}

void LockContent::onUserListChanged(QList<std::shared_ptr<User> > list)
{
    const bool allowShowUserSwitchButton = m_model->allowShowUserSwitchButton();
    const bool alwaysShowUserSwitchButton = m_model->alwaysShowUserSwitchButton();

    m_controlWidget->setUserSwitchEnable(alwaysShowUserSwitchButton || (allowShowUserSwitchButton && list.size() > 1));
}

void LockContent::tryGrabKeyboard()
{
#ifdef QT_DEBUG
    return;
#endif

    if (window()->windowHandle() && window()->windowHandle()->setKeyboardGrabEnabled(true)) {
        m_failures = 0;
        return;
    }

    m_failures++;

    if (m_failures == 15) {
        qDebug() << "Trying grabkeyboard has exceeded the upper limit. dde-lock will quit.";
        qApp->quit();
        return;
    }

    QTimer::singleShot(100, this, &LockContent::tryGrabKeyboard);
}

void LockContent::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape: {
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ConfirmPasswordMode)
            m_model->setAbortConfirm(false);
        break;
    }
    }
}
