/*
 * Copyright (C) 2018 ~ 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng@uniontech.com.so>
 *
 * Maintainer: fanpengcheng <fanpengcheng@uniontech.com.so>
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
#ifndef FRAMEDATABIND_H
#define FRAMEDATABIND_H

#include <QObject>
#include <functional>
#include <QVariant>

/**
 * @brief The FrameDataBind class 锁屏页面之间的数据同步
 * @note 之所以不用创建一份锁屏内容，然后跟随鼠标移动到对应屏幕上的方式，
 *       是因为对于多个分辨率不一样的屏幕，移动过去再显示需要resize，用户能看到变化，体验不好，优化这里需要注意这个问题
 */
class FrameDataBind : public QObject
{
    Q_OBJECT
public:
    static FrameDataBind *Instance();

    int registerFunction(const QString &flag, std::function<void (QVariant)> function);
    void unRegisterFunction(const QString &flag, int index);

    QVariant getValue(const QString &flag) const;
    void updateValue(const QString &flag, const QVariant &value);
    void clearValue(const QString &flag);

    void refreshData(const QString &flag);

private:
    FrameDataBind();
    ~FrameDataBind() override;
    FrameDataBind(const FrameDataBind &) = delete;

private:
    QMap<QString, QMap<int, std::function<void (QVariant)>>> m_registerList;
    QMap<QString, QVariant> m_datas;
};

#endif // FRAMEDATABIND_H
