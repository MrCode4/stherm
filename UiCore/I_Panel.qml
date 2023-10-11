import QtQuick

/*! ***********************************************************************************************
 * The I_Panel is the interface/base class that should be extended by 2D panel type Components.
 * ************************************************************************************************/
Rectangle {
    id: panel

    /* Property Declarations
     * ****************************************************************************************/
    property string                 title:      "<I_Panel>"

    /* Object Properties
     * ****************************************************************************************/
    // Invisible by default (shown on-demand)
    visible: false
}
