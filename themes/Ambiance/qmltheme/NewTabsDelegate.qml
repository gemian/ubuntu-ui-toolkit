/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem

Item {
    id: tabsDelegate
    anchors.fill: parent

    clip: true

    // TODO: move to style
    property bool swipeToSwitchTabs: true

    property VisualItemModel tabModel: item.__tabsModel

    Rectangle {
        id: orangebar
        color: "#c94212"
        anchors {
            left: parent.left
            right: parent.right
            top:parent.top
        }
        height: units.dp(2)
    }

    NewTabBar {
        id: tabBar
        anchors {
            top: orangebar.bottom
            left: parent.left
            right: parent.right
        }
        tabs: item
    }

    ListItem.Divider {
        id: divider
        anchors.top: tabBar.bottom
        height: units.gu(2)
    }

    ListView {
        id: tabView
        anchors {
            top: divider.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        interactive: parent.swipeToSwitchTabs
        model: tabsDelegate.tabModel
        onModelChanged: {
            print()
            tabView.updatePages();
        }
        currentIndex: item.selectedTabIndex
        onCurrentIndexChanged: {
            item.selectedTabIndex = tabView.currentIndex
        }

        orientation: ListView.Horizontal

        snapMode: ListView.SnapOneItem
        boundsBehavior: Flickable.DragOverBounds
        highlightFollowsCurrentItem: true
//        highlight: Item{}
        highlightRangeMode: ListView.StrictlyEnforceRange

        function updatePages() {
            var tab;
            if (!tabsDelegate.tabModel) return; // not initialized yet

            var tabList = tabsDelegate.tabModel.children
            for (var i=0; i < tabList.length; i++) {
                tab = tabList[i];
                tab.width = tabView.width;
                tab.height = tabView.height;
                tab.anchors.fill = undefined;
                if (tab.hasOwnProperty("__active")) tab.__active = true;
            }
            tabView.updateSelectedTabIndex();
        }

        onMovingChanged: {
            if(!moving) {
                // update the currentItem
                var relativePosition = contentX / tabsDelegate.width;
                // Clamping because on very narrow views contentX can overshoot
                tabView.currentIndex = MathUtils.clamp(Math.round(relativePosition), 0, tabModel.count-1);
                //item.selectedTabIndex = tabView.currentIndex;
            }
        }

        function updateSelectedTabIndex() {
            if (tabView.currentIndex === item.selectedTabIndex) return;
            // The view is automatically updated, because highlightFollowsCurrentItem
            tabView.currentIndex = item.selectedTabIndex;
        }

        Connections {
            target: item
            onSelectedTabIndexChanged: tabView.updateSelectedTabIndex()
        }

    }

    onWidthChanged: tabView.updatePages();
    onHeightChanged: tabView.updatePages();
    Component.onCompleted: tabView.updatePages();
}
