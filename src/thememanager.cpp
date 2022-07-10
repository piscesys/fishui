/*
 * Copyright (C) 2021 Piscesys Team.
 *
 * Author:     pisces <piscesys@foxmail.com>
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

#include "thememanager.h"

#include <QGuiApplication>

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusInterface>
#include <QDebug>

static const QString Service = "com.pisces.Settings";
static const QString ObjectPath = "/Theme";
static const QString Interface = "com.pisces.Theme";

ThemeManager::ThemeManager(QObject *parent) 
    : QObject(parent)
    , m_darkMode(false)
    , m_accentColorIndex(-1)
    , m_accentColor(m_blueColor) // The default is blue
    , m_fontSize(9.0)
    , m_fontFamily("Noto Sans")
{
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(Service, QDBusConnection::sessionBus(),
                                                                  QDBusServiceWatcher::WatchForRegistration);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [=] {
        initData();
        initDBusSignals();
    });

    initDBusSignals();
    initData();
}

qreal ThemeManager::devicePixelRatio() const
{
    return qApp->devicePixelRatio();
}

void ThemeManager::initData()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        m_darkMode = iface.property("isDarkMode").toBool();
        int accentColorID = iface.property("accentColor").toInt();
        setAccentColor(accentColorID);

        m_fontSize = iface.property("systemFontPointSize").toReal();
        emit fontSizeChanged();

        m_fontFamily = iface.property("systemFont").toString();
        emit fontFamilyChanged();

        emit darkModeChanged();
    }
}

void ThemeManager::initDBusSignals()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        QDBusConnection::sessionBus().connect(Service, ObjectPath, Interface, "darkModeChanged",
                                              this, SLOT(onDBusDarkModeChanged(bool)));
        QDBusConnection::sessionBus().connect(Service, ObjectPath, Interface, "accentColorChanged",
                                              this, SLOT(onDBusAccentColorChanged(int)));
        QDBusConnection::sessionBus().connect(Service, ObjectPath, Interface, "systemFontPointSizeChanged",
                                              this, SLOT(onDBusFontSizeChanged()));
        QDBusConnection::sessionBus().connect(Service, ObjectPath, Interface, "systemFontChanged",
                                              this, SLOT(onDBusFontFamilyChanged()));
    }
}

void ThemeManager::onDBusDarkModeChanged(bool darkMode)
{
    if (m_darkMode != darkMode) {
        m_darkMode = darkMode;
        emit darkModeChanged();
    }
}

void ThemeManager::onDBusAccentColorChanged(int accentColorID)
{
    setAccentColor(accentColorID);
}

void ThemeManager::onDBusFontSizeChanged()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    qreal size = iface.property("systemFontPointSize").toReal();
    if (size != m_fontSize) {
        m_fontSize = size;
        emit fontSizeChanged();
    }
}

void ThemeManager::onDBusFontFamilyChanged()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    QString family = iface.property("systemFont").toString();
    if (family != m_fontFamily) {
        m_fontFamily = family;
        emit fontFamilyChanged();
    }
}

void ThemeManager::setAccentColor(int accentColorID)
{
    if (m_accentColorIndex == accentColorID)
        return;

    m_accentColorIndex = accentColorID;

    switch (accentColorID) {
    case ACCENTCOLOR_BLUE:
        m_accentColor = m_blueColor;
        break;
    case ACCENTCOLOR_RED:
        m_accentColor = m_redColor;
        break;
    case ACCENTCOLOR_GREEN:
        m_accentColor = m_greenColor;
        break;
    case ACCENTCOLOR_PURPLE:
        m_accentColor = m_purpleColor;
        break;
    case ACCENTCOLOR_PINK:
        m_accentColor = m_pinkColor;
        break;
    case ACCENTCOLOR_ORANGE:
        m_accentColor = m_orangeColor;
        break;
    case ACCENTCOLOR_GREY:
        m_accentColor = m_greyColor;
        break;
    default:
        m_accentColor = m_blueColor;
        break;
    }

    emit accentColorChanged();
}
