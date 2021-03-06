/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef CALCULATE_UTIL_H
#define CALCULATE_UTIL_H

#include <QObject>
#include <QSize>
#include <QtCore>
#include <QGSettings>

#include "dbusinterface/dbuslauncher.h"

#define ALL_APPS            0
#define GROUP_BY_CATEGORY   1
#define SEARCH              2

class CalculateUtil : public QObject
{
    Q_OBJECT

signals:
    void layoutChanged() const;

public:
    static CalculateUtil *instance();

    static int calculateBesidePadding(const int screenWidth);

    inline int titleTextSize() const {return m_titleTextSize;}
    // NOTE: navgation text size animation max zoom scale is 1.2
    inline int navgationTextSize() const {return double(m_navgationTextSize) / 1.2;}
    inline int appColumnCount() const {return m_appColumnCount;}
    inline int appItemFontSize() const {return m_appItemFontSize;}
    inline QSize appIconSize() const
    { return QSize(m_appItemSize * m_appIconRadio, m_appItemSize * m_appIconRadio); }
    inline int appItemSpacing() const {return m_appItemSpacing;}
    inline QSize appItemSize() const {return QSize(m_appItemSize, m_appItemSize);}
    int displayMode() const;
    void setDisplayMode(const int mode);

    void increaseIconSize();
    void decreaseIconSize();
    inline void increaseItemSize() {m_appItemSize += 16;}
    inline void decreaseItemSize() {m_appItemSize -= 16;}

public slots:
    void calculateAppLayout(const QSize &containerSize, const int dockPosition);

private:
    explicit CalculateUtil(QObject *parent);
    void calculateTextSize(const int screenWidth);

private:
    static QPointer<CalculateUtil> INSTANCE;

    double m_appIconRadio = 0.4;
    int m_appItemFontSize = 12;
    int m_appItemSpacing = 10;
    int m_appItemSize = 130;
    int m_appColumnCount = 7;
    int m_navgationTextSize = 14;
    int m_titleTextSize = 15;

    DBusLauncher *m_launcherInter;
    QGSettings *m_launcherGsettings;
};

#endif // CALCULATE_UTIL_H
