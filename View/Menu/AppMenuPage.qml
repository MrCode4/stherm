import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AppMenuPage provides ui to access different section of application
 * ***********************************************************************************************/
BasePageView {
    id: root
    backButtonVisible: false
    header: null
    visible: false

    onVisibleChanged: {
        if (visible) {
            innerPage.y = 0
            innerPage.opacity = 1.0
        }
    }

    BasePageView {
        id: innerPage
        title: "Menu"
        y: height
        opacity: 0

        backButtonCallback: function() {
            if (root.StackView.view) {
                //! Then Page is inside an StackView
                if (root.StackView.view.currentItem === root) {
                    root.StackView.view.pop();
                }
            }
        }        

        //! Attach the animations
        //! Behaviour on y
        Behavior on y {
            NumberAnimation {
                from: height
                to: 0
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        //! Behaviour on opacity
        Behavior on opacity  {
            NumberAnimation {
                from: 0.0
                to: 1.0
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        contentItem: MenuListView {
            onMenuActivated: function(item) {
                let newProps = {};
                Object.assign(newProps, item.properties);
                Object.assign(newProps, {"uiSession": Qt.binding(() => uiSession)});
                root.StackView.view.push(item.view, newProps);
            }

            model: [
                {
                    icon: FAIcons.sunDust,
                    text: "System Mode",
                    view: "qrc:/Stherm/View/SystemModePage.qml"
                },
                {
                    icon: FAIcons.wifi,
                    text: "Wi-Fi Settings",
                    view: "qrc:/Stherm/View/WifiPage.qml"
                },
                {
                    icon: FAIcons.bolt,
                    text: "Backlight",
                    view: "qrc:/Stherm/View/BacklightPage.qml"
                },
                {
                    icon: FAIcons.calendarDays,
                    text: "Schedule",
                    view: "qrc:/Stherm/View/ScheduleView.qml"
                },
                {
                    icon: FAIcons.gear,
                    text: "Settings",
                    view: "qrc:/Stherm/View/Menu/SettingsMenuPage.qml"
                },
                {
                    icon: FAIcons.headSet,
                    text: "Contact Contractor",
                    view: "qrc:/Stherm/View/ContactContractorPage.qml"
                },
                {
                    icon: FAIcons.message_middle,
                    text: "Messages",
                    view: "qrc:/Stherm/View/Menu/MessagesPage.qml"
                }
            ]
        }
    }
}
