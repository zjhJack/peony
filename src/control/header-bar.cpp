/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "header-bar.h"
#include "main-window.h"

#include "view-type-menu.h"
#include "sort-type-menu.h"
#include "operation-menu.h"

#include "directory-view-container.h"
#include "directory-view-widget.h"

#include <QHBoxLayout>
#include <advanced-location-bar.h>

#include <QStyleOptionToolButton>

#include <QDebug>

static HeaderBarStyle *global_instance = nullptr;

HeaderBar::HeaderBar(MainWindow *parent) : QToolBar(parent)
{
    setStyle(HeaderBarStyle::getStyle());

    m_window = parent;
    //disable default menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    //setAttribute(Qt::WA_OpaquePaintEvent);
    setStyleSheet(".HeaderBar{"
                  "background-color: transparent;"
                  "border: 0px solid transparent;"
                  "margin: 4px 5px 4px 5px;"
                  "};");

    setMovable(false);

    auto a = addAction(QIcon::fromTheme("folder-new-symbolic"), tr("Create Folder"), [=](){
        //FIXME:
    });
    auto createFolder = qobject_cast<QToolButton *>(widgetForAction(a));
    createFolder->setAutoRaise(false);
    createFolder->setFixedSize(QSize(40, 40));
    createFolder->setIconSize(QSize(16, 16));

    a = addAction(QIcon::fromTheme("terminal-app-symbolic"), tr("Open Terminal"), [=](){
        //FIXME:
    });
    auto openTerminal = qobject_cast<QToolButton *>(widgetForAction(a));
    openTerminal->setAutoRaise(false);
    openTerminal->setFixedSize(QSize(40, 40));
    openTerminal->setIconSize(QSize(16, 16));

    addSpacing(9);

    auto goBack = new HeadBarPushButton(this);
    m_go_back = goBack;
    goBack->setEnabled(false);
    goBack->setToolTip(tr("Go Back"));
    goBack->setFixedSize(QSize(36, 28));
    goBack->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    addWidget(goBack);
    connect(goBack, &QPushButton::clicked, m_window, [=](){
        m_window->getCurrentPage()->goBack();
    });

    auto goForward = new HeadBarPushButton(this);
    m_go_forward = goForward;
    goForward->setEnabled(false);
    goForward->setToolTip(tr("Go Forward"));
    goForward->setFixedSize(QSize(36, 28));
    goForward->setIcon(QIcon::fromTheme("go-next-symbolic"));
    addWidget(goForward);
    connect(goForward, &QPushButton::clicked, m_window, [=](){
        m_window->getCurrentPage()->goForward();
    });

    addSpacing(9);

    auto locationBar = new Peony::AdvancedLocationBar(this);
    m_location_bar = locationBar;
    addWidget(locationBar);

    connect(m_location_bar, &Peony::AdvancedLocationBar::updateWindowLocationRequest, this, &HeaderBar::updateLocationRequest);

    addSpacing(9);
    a = addAction(QIcon::fromTheme("edit-find-symbolic"), tr("Search"), [=](){
        //FIXME:
    });
    auto search = qobject_cast<QToolButton *>(widgetForAction(a));
    search->setAutoRaise(false);
    search->setFixedSize(QSize(40, 40));
    setIconSize(QSize(16, 16));

    addSpacing(9);

    a = addAction(QIcon::fromTheme("view-grid-symbolic"), tr("View Type"));
    auto viewType = qobject_cast<QToolButton *>(widgetForAction(a));
    viewType->setAutoRaise(false);
    viewType->setFixedSize(QSize(57, 40));
    viewType->setIconSize(QSize(16, 16));
    viewType->setPopupMode(QToolButton::InstantPopup);

    m_view_type_menu = new ViewTypeMenu(viewType);
    viewType->setMenu(m_view_type_menu);

    connect(m_view_type_menu, &ViewTypeMenu::switchViewRequest, this, [=](const QString &id, const QIcon &icon){
        viewType->setText(id);
        viewType->setIcon(icon);
        this->viewTypeChangeRequest(id);
    });

    a = addAction(QIcon::fromTheme("view-sort-ascending-symbolic"), tr("Sort Type"));
    auto sortType = qobject_cast<QToolButton *>(widgetForAction(a));
    sortType->setAutoRaise(false);
    sortType->setFixedSize(QSize(57, 40));
    sortType->setIconSize(QSize(16, 16));
    sortType->setPopupMode(QToolButton::InstantPopup);

    m_sort_type_menu = new SortTypeMenu(this);
    sortType->setMenu(m_sort_type_menu);

    connect(m_sort_type_menu, &SortTypeMenu::switchSortTypeRequest, m_window, &MainWindow::setCurrentSortColumn);
    connect(m_sort_type_menu, &SortTypeMenu::switchSortOrderRequest, m_window, [=](Qt::SortOrder order){
        if (order == Qt::AscendingOrder) {
            sortType->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
        } else {
            sortType->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
        }
        m_window->setCurrentSortOrder(order);
    });
    connect(m_sort_type_menu, &QMenu::aboutToShow, m_sort_type_menu, [=](){
        m_sort_type_menu->setSortType(m_window->getCurrentSortColumn());
        m_sort_type_menu->setSortOrder(m_window->getCurrentSortOrder());
    });

    a = addAction(QIcon::fromTheme("open-menu-symbolic"), tr("Option"));
    auto popMenu = qobject_cast<QToolButton *>(widgetForAction(a));
    popMenu->setAutoRaise(false);
    popMenu->setFixedSize(QSize(57, 40));
    popMenu->setIconSize(QSize(16, 16));
    popMenu->setPopupMode(QToolButton::InstantPopup);

    m_operation_menu = new OperationMenu(m_window, this);
    popMenu->setMenu(m_operation_menu);

    //minimize, maximize and close
    a = addAction(QIcon::fromTheme("window-minimize-symbolic"), tr("Minimize"), [=](){
        m_window->showMinimized();
    });
    auto minimize = qobject_cast<QToolButton *>(widgetForAction(a));
    minimize->setAutoRaise(false);
    minimize->setFixedSize(QSize(40, 40));
    minimize->setIconSize(QSize(16, 16));

    //window-maximize-symbolic
    //window-restore-symbolic
    a = addAction(QIcon::fromTheme("window-maximize-symbolic"), nullptr, [=](){
        m_window->maximizeOrRestore();
    });
    auto maximizeAndRestore = qobject_cast<QToolButton *>(widgetForAction(a));
    m_maximize_restore_button = maximizeAndRestore;
    maximizeAndRestore->setAutoRaise(false);
    maximizeAndRestore->setFixedSize(QSize(40, 40));
    maximizeAndRestore->setIconSize(QSize(16, 16));

    a = addAction(QIcon::fromTheme("window-close-symbolic"), tr("Close"), [=](){
        m_window->close();
    });
    auto close = qobject_cast<QToolButton *>(widgetForAction(a));
    close->setAutoRaise(false);
    close->setFixedSize(QSize(40, 40));
    close->setIconSize(QSize(16, 16));
    connect(close, &QToolButton::clicked, this, [=](){
        m_window->close();
    });
}

void HeaderBar::addSpacing(int pixel)
{
    for (int i = 0; i < pixel; i++) {
        addSeparator();
    }
}

void HeaderBar::setLocation(const QString &uri)
{
    m_location_bar->updateLocation(uri);
}

void HeaderBar::updateIcons()
{
    qDebug()<<m_window->getCurrentUri();
    qDebug()<<m_window->getCurrentSortColumn();
    qDebug()<<m_window->getCurrentSortOrder();
    m_view_type_menu->setCurrentView(m_window->getCurrentPage()->getView()->viewId());
    m_sort_type_menu->switchSortTypeRequest(m_window->getCurrentSortColumn());
    m_sort_type_menu->switchSortOrderRequest(m_window->getCurrentSortOrder());

    //go back & go forward
    m_go_back->setEnabled(m_window->getCurrentPage()->canGoBack());
    m_go_forward->setEnabled(m_window->getCurrentPage()->canGoForward());

    //maximize & restore
    updateMaximizeState();
}

void HeaderBar::updateMaximizeState()
{
    //maximize & restore
    bool maximized = m_window->isMaximized();
    if (maximized) {
        m_maximize_restore_button->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    } else {
        m_maximize_restore_button->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    }
}

//HeaderBarToolButton
HeaderBarToolButton::HeaderBarToolButton(QWidget *parent) : QToolButton(parent)
{
    setAutoRaise(false);
    setIconSize(QSize(16, 16));
}

//HeadBarPushButton
HeadBarPushButton::HeadBarPushButton(QWidget *parent) : QPushButton(parent)
{
    setIconSize(QSize(16, 16));
}

//HeaderBarStyle
HeaderBarStyle *HeaderBarStyle::getStyle()
{
    if (!global_instance) {
        global_instance = new HeaderBarStyle;
    }
    return global_instance;
}

int HeaderBarStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ToolBarIconSize:
        return 16;
    case PM_ToolBarSeparatorExtent:
        return 1;
    case PM_ToolBarItemSpacing: {
        return 1;
    }
    case PM_ToolBarItemMargin:
    case PM_ToolBarFrameWidth:
        return 0;
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

void HeaderBarStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    //This is a "lie". We want to use instant popup menu for tool button, and we aslo
    //want use popup menu style with this tool button, so we change the related flags
    //to draw in our expected.
    if (control == CC_ToolButton) {
        QStyleOptionToolButton button = *qstyleoption_cast<const QStyleOptionToolButton *>(option);
        if (button.features.testFlag(QStyleOptionToolButton::HasMenu)) {
            button.features = QStyleOptionToolButton::None;
            button.features |= QStyleOptionToolButton::HasMenu;
            button.features |= QStyleOptionToolButton::MenuButtonPopup;
            button.subControls |= QStyle::SC_ToolButtonMenu;
            return QProxyStyle::drawComplexControl(control, &button, painter, widget);
        }
    }
    return QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void HeaderBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == PE_IndicatorToolBarSeparator) {
        return;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}