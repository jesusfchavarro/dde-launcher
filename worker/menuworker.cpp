/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "menuworker.h"

AppsManager *MenuWorker::m_appManager = nullptr;




MenuWorker::MenuWorker(QObject *parent) : QObject(parent)
{
    qDebug() << "MenuWorker";
    m_launcherInterface = new DBusLauncher(this);
    m_menuManagerInterface = new DBusMenuManager(this);
    m_dockAppManagerInterface = new DBusDockedAppManager(this);
    m_startManagerInterface = new DBusStartManager(this);
//    m_notifcationInterface = new NotificationInterface(NotificationInterface::staticServiceName(),
//                                                       NotificationInterface::staticObjectPathName(),
//                                                       QDBusConnection::sessionBus(),
//                                                       this);
    m_menuInterface = NULL;
    if (!m_appManager)
        m_appManager = AppsManager::instance(this);

    initConnect();


}


void MenuWorker::initConnect(){
//    connect(signalManager, SIGNAL(contextMenuShowed(QString,QPoint)),
//            this, SLOT(showMenuByAppItem(QString,QPoint)));
//    connect(signalManager, SIGNAL(appOpened(QString)), this, SLOT(handleOpen(QString)));
//    connect(signalManager, SIGNAL(uninstallActionChanged(QString,int)), this, SLOT(handleUninstallAction(QString,int)));
//    connect(signalManager, SIGNAL(contextMenuHided(QString)), this, SLOT(hideMenuByAppKey(QString)));
}

MenuWorker::~MenuWorker()
{
}


void MenuWorker::showMenuByAppItem(QString appKey, QPoint pos){
    m_appKeyRightClicked = appKey;
    qDebug() << "appKey" << appKey;
    QString menuContent = createMenuContent(appKey);
    QString menuJsonContent = JsonToQString(pos, menuContent);
    QString menuDBusObjectpath = registerMenu();
    qDebug() << "dbus objectpath:" << menuDBusObjectpath;
    if (menuDBusObjectpath.length() > 0){
        showMenu(menuDBusObjectpath, menuJsonContent);
        m_currentMenuObjectPath = menuDBusObjectpath;
        m_menuObjectPaths.insert(appKey, menuDBusObjectpath);
    }else{
        qCritical() << "register menu fail!";
    }
}

QString MenuWorker::createMenuContent(QString appKey){
    m_isItemOnDesktop = isItemOnDesktop(appKey);
    m_isItemOnDock = isItemOnDock(appKey);
    m_isItemStartup = isItemStartup(appKey);

    QJsonObject openObj = createMenuItem(0, tr("Open(_O)"));
    QJsonObject seperatorObj1 = createSeperator();
    QJsonObject desktopObj;
    if (m_isItemOnDesktop){
        desktopObj = createMenuItem(1, tr("Remove from desktop"));
    }else{
        desktopObj = createMenuItem(1, tr("Send to desktop(_E)"));
    }
    QJsonObject dockObj;
    if (m_isItemOnDock){
        dockObj = createMenuItem(2, tr("Remove from dock"));
    }else{
        dockObj = createMenuItem(2, tr("Send to dock(_C)"));
    }
    QJsonObject seperatorObj2 = createSeperator();
    QJsonObject startupObj;
    if (m_isItemStartup){
        startupObj = createMenuItem(3, tr("Remove from startup(_R)"));
    }else{
        startupObj = createMenuItem(3, tr("Add to startup(_A)"));
    }
    QJsonObject uninstallObj = createMenuItem(4, tr("Uninstall"));

    QJsonArray items;
    items.append(openObj);
    items.append(seperatorObj1);
    items.append(desktopObj);
    items.append(dockObj);
    items.append(seperatorObj2);
    items.append(startupObj);
    items.append(uninstallObj);

    QJsonObject menuObj;
    menuObj["checkableMenu"] = false;
    menuObj["singleCheck"] = false;
    menuObj["items"] = items;

    return QString(QJsonDocument(menuObj).toJson());
}

QJsonObject MenuWorker::createMenuItem(int itemId, QString itemText){
    QJsonObject itemObj;
    itemObj["itemId"] = QString::number(itemId);
    itemObj["itemText"] = itemText;
    itemObj["isActive"] = true;
    itemObj["isCheckable"] = false;
    itemObj["checked"] = false;
    itemObj["itemIcon"] = "";
    itemObj["itemIconHover"] = "";
    itemObj["itemIconInactive"] = "";
    itemObj["showCheckMark"] = false;
    QJsonObject subMenuObj;
    subMenuObj["checkableMenu"] = false;
    subMenuObj["singleCheck"] = false;
    subMenuObj["items"] = QJsonArray();
    itemObj["itemSubMenu"] = subMenuObj;
    return itemObj;
}


QJsonObject MenuWorker::createSeperator(){
    return createMenuItem(-100, "");
}


bool MenuWorker::isItemOnDesktop(QString appKey){
    bool flag = false;
    QDBusPendingReply<bool> reply = m_launcherInterface->IsItemOnDesktop(appKey);
    reply.waitForFinished();
    if (!reply.isError()){
        flag = reply.argumentAt(0).toBool();
    } else {
        qCritical() << reply.error().name() << reply.error().message();
    }
    qDebug() << appKey << flag;
    return flag;
}

bool MenuWorker::isItemOnDock(QString appKey){
    bool flag = false;
    QDBusPendingReply<bool> reply = m_dockAppManagerInterface->IsDocked(appKey);
    reply.waitForFinished();
    if (!reply.isError()){
        flag = reply.argumentAt(0).toBool();
    } else {
        qCritical() << reply.error().name() << reply.error().message();
    }
    return flag;
}

bool MenuWorker::isItemStartup(QString appKey){
    QString desktopUrl= m_appManager->getItemInfo(appKey).m_desktop;
    bool flag = m_appManager->appIsAutoStart(desktopUrl);
    return flag;
}


QString MenuWorker::JsonToQString(QPoint pos, QString menucontent) {
    QJsonObject menuObj;
    menuObj["x"] = pos.x();
    menuObj["y"] = pos.y();
    menuObj["isDockMenu"] = false;
    menuObj["menuJsonContent"] = menucontent;
    return QString(QJsonDocument(menuObj).toJson());
}

QString MenuWorker::registerMenu() {
    QDBusPendingReply<QDBusObjectPath> reply = m_menuManagerInterface->RegisterMenu();
    reply.waitForFinished();
    if (!reply.isError()) {
        return reply.value().path();
    } else {
        qDebug() << "reply:" << reply.error().message();
        return "";
    }
}

void MenuWorker::showMenu(QString menuDBusObjectPath, QString menuContent) {
    qDebug() << menuDBusObjectPath;
    m_menuInterface = new DBusMenu(MenuManager_service, menuDBusObjectPath, QDBusConnection::sessionBus(), this);
    m_menuInterface->ShowMenu(menuContent);
    connect(m_menuInterface, SIGNAL(ItemInvoked(QString, bool)),this, SLOT(menuItemInvoked(QString,bool)));
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), this, SLOT(handleMenuClosed()));
    connect(m_menuInterface, SIGNAL(MenuUnregistered()), m_menuInterface, SLOT(deleteLater()));
}

void MenuWorker::hideMenu(const QString &menuDBusObjectPath)
{
    m_menuManagerInterface->UnregisterMenu(menuDBusObjectPath);
}

void MenuWorker::hideMenuByAppKey(const QString &appKey)
{
    if (m_menuObjectPaths.contains(appKey)){
        hideMenu(m_menuObjectPaths.value(appKey));
    }
}

void MenuWorker::menuItemInvoked(QString itemId, bool flag){
    Q_UNUSED(flag)
    int id = itemId.toInt();
    qDebug() << "menuItemInvoked" << itemId;
    switch (id) {
    case 0:
        handleOpen(m_appKeyRightClicked);
        break;
    case 1:
        handleToDesktop(m_appKeyRightClicked);
        break;
    case 2:
        handleToDock(m_appKeyRightClicked);
        break;
    case 3:
        handleToStartup(m_appKeyRightClicked);
        break;
    case 4:
//        emit signalManager->appUninstalled(m_appKeyRightClicked);
        break;
    default:
        break;
    }
}


void MenuWorker::handleOpen(QString appKey){
//    emit signalManager->Hide();
  QString url = m_appManager->getItemInfo(appKey).m_desktop;
    qDebug() << "handleOpen" << appKey << url;
    uint timestamp = QX11Info::getTimestamp();
    QDBusPendingReply<bool> reply = m_startManagerInterface->LaunchWithTimestamp(url, timestamp);
    reply.waitForFinished();
    if (!reply.isError()) {
        bool ret = reply.argumentAt(0).toBool();
        qDebug() << "Launch app:" << ret;
        if (ret){
             m_launcherInterface->MarkLaunched(appKey);
//            dbusController->getLauncherInterface()->RecordFrequency(appKey);
//            emit signalManager->newinstalllindicatorHided(appKey);
        }
    } else {
        qCritical() << reply.error().name() << reply.error().message();
    }
}

void MenuWorker::handleMenuClosed(){
//    emit signalManager->rightClickedChanged(false);
}

void MenuWorker::handleToDesktop(QString appKey){
    qDebug() << "handleToDesktop" << appKey;
    if (m_isItemOnDesktop){
        QDBusPendingReply<bool> reply = m_launcherInterface->RequestRemoveFromDesktop(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from desktop:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply = m_launcherInterface->RequestSendToDesktop(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "send to desktop:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }
}

void MenuWorker::handleToDock(QString appKey){
    qDebug() << "handleToDock" << appKey;
    if (m_isItemOnDock){
        QDBusPendingReply<bool> reply = m_dockAppManagerInterface->RequestUndock(appKey);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from dock:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply =  m_dockAppManagerInterface->ReqeustDock(appKey, "", "", "");
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "send to dock:" << ret;
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }
}

void MenuWorker::handleToStartup(QString appKey){
   QString url = "";// dbusController->getItemInfo(appKey).url;
    qDebug() << "handleToStartup" << appKey << url;

    if (m_isItemStartup){
        QDBusPendingReply<bool> reply = m_startManagerInterface->RemoveAutostart(url);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "remove from startup:" << ret;
            if (ret) {
//                emit signalManager->hideAutoStartLabel(appKey);
            }
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }else{
        QDBusPendingReply<bool> reply =  m_startManagerInterface->AddAutostart(url);
        reply.waitForFinished();
        if (!reply.isError()) {
            bool ret = reply.argumentAt(0).toBool();
            qDebug() << "add to startup:" << ret;
            if (ret){
//                emit signalManager->showAutoStartLabel(appKey);
            }
        } else {
            qCritical() << reply.error().name() << reply.error().message();
        }
    }
}

void MenuWorker::handleUninstallAction(QString appKey, int id){
    qDebug() << sender() << "handleUninstallAction" << appKey << id;
    switch (id) {
    case 0:
//        if (LauncherApp::UnistallAppNames.contains(appKey)){
//            LauncherApp::UnistallAppNames.remove(appKey);
//        }
        break;
    case 1:
        startUnistall(appKey);
        break;
    default:
        break;
    }

}

void MenuWorker::startUnistall(QString appKey){
    QDBusPendingReply<> reply = m_launcherInterface->RequestUninstall(appKey, false);
    reply.waitForFinished();
    if (!reply.isError()) {
        qDebug() << "unistall function excute finished!";
    } else {
        qDebug() << "unistall action fail";
    }
}

void MenuWorker::handleUninstallSuccess(const QString &appKey){
//    emit signalManager->itemDeleted(appKey);
}

void MenuWorker::handleUninstallFail(const QString &appKey, const QString &message){
    qDebug() << "handleUninstallFail" << appKey << message;
}
