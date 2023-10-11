import QtQuick
import QtQuick.Controls
import QTemplateProject
import RoniaKit

/*! ***********************************************************************************************
 * Context Menu
 * ************************************************************************************************/
Menu {
    id: contextMenu

    /* Property Declarations
     * ****************************************************************************************/
    property Dashboard dashboard

    /* Object Properties
     * ****************************************************************************************/
    width: 180
    height: 250

    //background is overrided
    background: Rectangle{
        anchors.fill: parent
        radius: 5
        color: "#262626"
        border.width: 1
        border.color: "#1c1c1c"
    }

    /* Children
     * ****************************************************************************************/

    //! Simple Gauge
    ContextMenuItem {
        id: addModernGauge
        title: "Add Simple Gauge"
        onClicked: {
            var control = AppCore.createControl();
            control.position = Qt.vector2d(contextMenu.x, contextMenu.y);
            control.type = AppSpec.ControlType.SimpleGauge
            dashboard.addControl(control);
        }
    }

    //! Speed Gauge
    ContextMenuItem {
        id: addSpeedGauge
        title: "Add Speed Gauge"
        onClicked: {
            var control = AppCore.createControl();
            control.position = Qt.vector2d(contextMenu.x, contextMenu.y);
            control.type = AppSpec.ControlType.SpeedGauge
            dashboard.addControl(control);
        }
    }

    ContextMenuItem {
        id: addLevelGauge
        title: "Add Level Gauge"
        onClicked: {
            var control = AppCore.createControl();
            control.position = Qt.vector2d(contextMenu.x, contextMenu.y);
            control.type = AppSpec.ControlType.LevelGauge
            dashboard.addControl(control);
        }
    }

    //! Graph
    ContextMenuItem {
        id: graphMenuItem
        title: "Add Graph"
        onClicked: {
            var control = AppCore.createControl();
            control.position = Qt.vector2d(contextMenu.x, contextMenu.y);
            control.type = AppSpec.ControlType.Graph
            dashboard.addControl(control);
        }
    }

    //! Ramps Sequencer
    ContextMenuItem {
        id: rampMenuItem
        title: "Add RampSequencer"
        onClicked: {
            var control = AppCore.createControl();
            control.position = Qt.vector2d(contextMenu.x, contextMenu.y);
            control.type = AppSpec.ControlType.RampSequencer
            dashboard.addControl(control);
        }
    }

    //! Motor State Info
    ContextMenuItem {
        id: motorStateItem
        title: "Add Motor State"
        onClicked: {
            var control = AppCore.createControl();
            control.position = Qt.vector2d(contextMenu.x, contextMenu.y);
            control.type = AppSpec.ControlType.MotorStateInformation
            dashboard.addControl(control);
        }
    }
}
