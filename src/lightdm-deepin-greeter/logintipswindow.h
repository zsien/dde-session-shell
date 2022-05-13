#ifndef LOGINTIPSWINDOW_H
#define LOGINTIPSWINDOW_H

#include "fullscreenbackground.h"
#include "public_func.h"

#include <QWidget>

class QLabel;
class QPushButton;
class QHBoxLayout;
class QPalette;

class LoginTipsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginTipsWindow(QWidget *parent = 0);
    bool isValid();

signals:
    void requestClosed();

private:
    void initUI();

private:
    QHBoxLayout *m_mainLayout;
    QLabel *m_tipLabel;
    QLabel *m_contentLabel;
    QPushButton *m_btn;
    QString m_tips;
    QString m_content;

};

#endif // LOGINTIPSWINDOW_H
