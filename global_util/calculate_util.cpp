/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "calculate_util.h"
#include "dbusinterface/monitorinterface.h"
#include "dbusinterface/dbusdisplay.h"

#include <QDebug>
#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>

static const QString DisplayModeKey = "display-mode";
static const QString DisplayModeFree = "free";
static const QString DisplayModeCategory = "category";

QPointer<CalculateUtil> CalculateUtil::INSTANCE = nullptr;

CalculateUtil *CalculateUtil::instance()
{
    if (INSTANCE.isNull())
        INSTANCE = new CalculateUtil(nullptr);

    return INSTANCE;
}

int CalculateUtil::calculateBesidePadding(const int screenWidth)
{
    // static const int NAVIGATION_WIDGET_WIDTH = 180;
    if (screenWidth > 1366)
        return 180;

    return 130;
}

void CalculateUtil::setDisplayMode(const int mode)
{
    m_launcherGsettings->set(DisplayModeKey, mode == ALL_APPS ? DisplayModeFree : DisplayModeCategory);
}

void CalculateUtil::increaseIconSize()
{
    m_appIconRadio = std::min(0.7, m_appIconRadio + 0.1);
}

void CalculateUtil::decreaseIconSize()
{
    m_appIconRadio = std::max(0.3, m_appIconRadio - 0.1);
}

int CalculateUtil::displayMode() const
{
    const QString displayMode = m_launcherGsettings->get(DisplayModeKey).toString();

    if (displayMode == DisplayModeCategory) {
        return GROUP_BY_CATEGORY;
    }

    return ALL_APPS;
}

void CalculateUtil::calculateAppLayout(const QSize &containerSize, const int dockPosition)
{
    Q_UNUSED(dockPosition);

    // mini mode
    if (!m_launcherInter->fullscreen())
    {
        m_appItemSpacing = 6;
        m_appItemSize = 120;
        m_appItemFontSize = 8;
        m_appColumnCount = displayMode() == ALL_APPS ? 4 : 1;

        emit layoutChanged();
        return;
    }

    const QRect pr = qApp->primaryScreen()->geometry();
    const int screenWidth = pr.width();
    const int remain_width = screenWidth - calculateBesidePadding(screenWidth) * 2;

    const int itemWidth = 210;
    const int spacing = 15;
    const int columns = remain_width / itemWidth;

    const int calc_item_width = (double(containerSize.width()) - spacing * columns * 2) / columns + 0.5;
    const int calc_spacing = (double(containerSize.width()) - calc_item_width * columns) / (columns * 2) - 1;


    calculateTextSize(screenWidth);

    m_appItemSpacing = calc_spacing;
    m_appItemSize = calc_item_width;
    m_appColumnCount = columns;

    // calculate font size;
    m_appItemFontSize = m_appItemSize <= 80 ? 8 : qApp->font().pointSize();

    emit layoutChanged();
}

CalculateUtil::CalculateUtil(QObject *parent)
    : QObject(parent),
      m_launcherGsettings(new QGSettings("com.deepin.dde.launcher",
                                         "/com/deepin/dde/launcher/", this))
{
    m_launcherInter = new DBusLauncher(this);
}

void CalculateUtil::calculateTextSize(const int screenWidth)
{
    if (screenWidth > 1366)
    {
        m_navgationTextSize = 14;
        m_titleTextSize = 15;
    } else {
        m_navgationTextSize = 11;
        m_titleTextSize = 13;
    }
}
