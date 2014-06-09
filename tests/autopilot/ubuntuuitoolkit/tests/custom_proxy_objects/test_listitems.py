# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
#
# Copyright (C) 2013, 2014 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import ubuntuuitoolkit
from ubuntuuitoolkit import listitems, tests


class SwipeToDeleteTestCase(tests.QMLStringAppTestCase):

    test_qml = ("""
import QtQuick 2.0
import Ubuntu.Components 0.1
import Ubuntu.Components.ListItems 0.1


MainView {
    width: units.gu(48)
    height: units.gu(60)

    Page {

        ListModel {
            id: testModel

            ListElement {
                name: "listitem_destroyed_on_remove_with_confirm"
                label: "Item destroyed on remove with confirmation"
                confirm: true
            }
            ListElement {
                name: "listitem_destroyed_on_remove_without_confirm"
                label: "Item destroyed on remove without confirmation"
                confirm: false
            }
        }

        Column {
            anchors { fill: parent }

            Standard {
                objectName: "listitem_standard"
                confirmRemoval: true
                removable: true
                text: 'Slide to remove'
            }

            Empty {
                objectName: "listitem_empty"
            }

            Standard {
                objectName: "listitem_without_confirm"
                confirmRemoval: false
                removable: true
                text: "Item without delete confirmation"
            }

            ListView {
                anchors { left: parent.left; right: parent.right }
                height: childrenRect.height
                model: testModel

                delegate: Standard {
                    removable: true
                    confirmRemoval: confirm
                    onItemRemoved: testModel.remove(index)
                    text: label
                    objectName: name
                }
            }
        }
    }
}
""")

    def setUp(self):
        super(SwipeToDeleteTestCase, self).setUp()
        self._item = self.main_view.select_single(
            listitems.Standard, objectName='listitem_standard')
        self.assertTrue(self._item.exists())

    def test_supported_class(self):
        self.assertTrue(issubclass(
            listitems.Base, listitems.Empty))
        self.assertTrue(issubclass(
            listitems.ItemSelector, listitems.Empty))
        self.assertTrue(issubclass(
            listitems.Standard, listitems.Empty))
        self.assertTrue(issubclass(
            listitems.SingleControl, listitems.Empty))
        self.assertTrue(issubclass(
            listitems.MultiValue, listitems.Base))
        self.assertTrue(issubclass(
            listitems.SingleValue, listitems.Base))
        self.assertTrue(issubclass(
            listitems.Subtitled, listitems.Base))

    def test_standard_custom_proxy_object(self):
        self.assertIsInstance(self._item, listitems.Standard)

    def test_swipe_item(self):
        self._item.swipe_to_delete()
        self.assertTrue(self._item.waitingConfirmationForRemoval)

    def test_swipe_item_to_right(self):
        self._item.swipe_to_delete('right')
        self.assertTrue(self._item.waitingConfirmationForRemoval)

    def test_swipe_item_to_left(self):
        # This will do a right to left swipe behind the scenes
        self._item.swipe_to_delete('left')
        self.assertTrue(self._item.waitingConfirmationForRemoval)

    def test_swipe_item_to_wrong_direction(self):
        self.assertRaises(
            ubuntuuitoolkit.ToolkitException,
            self._item.swipe_to_delete, 'up')

    def test_delete_item_moving_right(self):
        self._item.swipe_to_delete('right')
        self._item.confirm_removal()
        self.assertFalse(self._item.exists())

    def test_delete_item_moving_left(self):
        # This will do a right to left swipe behind the scenes
        self._item.swipe_to_delete('left')
        self._item.confirm_removal()
        self.assertFalse(self._item.exists())

    def test_delete_non_removable_item(self):
        self._item = self.main_view.select_single(
            listitems.Empty, objectName='listitem_empty')
        self.assertRaises(
            ubuntuuitoolkit.ToolkitException, self._item.swipe_to_delete)

    def test_confirm_removal_when_item_was_not_swiped(self):
        self.assertRaises(
            ubuntuuitoolkit.ToolkitException, self._item.confirm_removal)

    def test_delete_item_without_confirm(self):
        item = self.main_view.select_single(
            listitems.Standard, objectName='listitem_without_confirm')
        item.swipe_to_delete()
        self.assertFalse(item.exists())

    def test_delete_item_with_confirmation_that_will_be_destroyed(self):
        item = self.main_view.select_single(
            listitems.Standard,
            objectName='listitem_destroyed_on_remove_with_confirm')
        item.swipe_to_delete()
        item.confirm_removal()
        self.assertFalse(item.exists())

    def test_delete_item_without_confirmation_that_will_be_destroyed(self):
        item = self.main_view.select_single(
            listitems.Standard,
            objectName='listitem_destroyed_on_remove_without_confirm')
        item.swipe_to_delete()
        self.assertFalse(item.exists())
