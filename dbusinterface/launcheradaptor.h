/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -N -c LauncherAdaptor -a launcheradaptor launcher.xml
 *
 * qdbusxml2cpp is Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef LAUNCHERADAPTOR_H_1440725376
#define LAUNCHERADAPTOR_H_1440725376

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.dde.Launcher
 */
class LauncherAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.Launcher")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.dde.Launcher\">\n"
"    <method name=\"Exit\"/>\n"
"    <method name=\"Show\"/>\n"
"    <method name=\"Hide\"/>\n"
"    <method name=\"Toggle\"/>\n"
"    <signal name=\"Closed\"/>\n"
"    <signal name=\"Shown\"/>\n"
"  </interface>\n"
        "")
public:
    LauncherAdaptor(QObject *parent);
    virtual ~LauncherAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void Exit();
    void Hide();
    void Show();
    void Toggle();
Q_SIGNALS: // SIGNALS
    void Closed();
    void Shown();
};

#endif