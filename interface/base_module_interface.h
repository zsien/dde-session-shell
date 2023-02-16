// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

#include <QtCore>

namespace dss {
namespace module {

const QStringList ValidVersions = {"1.0.0", "1.0.1"};
class BaseModuleInterface
{
public:
    /**
     * @brief The ModuleType enum
     * 模块的类型
     */
    enum ModuleType {
        LoginType,
        TrayType
    };

    virtual ~BaseModuleInterface() = default;

    /**
     * @brief 界面相关的初始化
     * 插件在非主线程加载，故界面相关的初始化需要放在这个方法里，由主程序调用并初始化
     */
    virtual void init() = 0;

    /**
     * @brief 键值，用于与其它模块区分
     * @return QString
     */
    virtual QString key() const = 0;

    /**
     * @brief 模块想显示的界面
     * content 对象由模块自己管理
     * @return QWidget*
     */
    virtual QWidget *content() = 0;

    /**
     * @brief 模块的类型
     * @return ModuleType
     */
    virtual ModuleType type() const = 0;

    /**
     * @brief isNeedInitPlugin 是否需要初始化插件，主要适应插件通过配置不显示等情况。
     * @return
     */
    virtual bool isNeedInitPlugin() const = 0;
};

} // namespace module
} // namespace dss

Q_DECLARE_INTERFACE(dss::module::BaseModuleInterface, "com.deepin.dde.shell.Modules")

#endif // MODULE_INTERFACE_H
