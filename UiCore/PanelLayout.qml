import QtQuick 2.0

/*! ***********************************************************************************************
 * The PanelLayout listens to the UiSession and shows any panels that are stacked. We use a stacked
 * (first-in-last-out) approach.
 * ************************************************************************************************/
Rectangle {
    id: panelLayout

    /* Property Declarations
     * ****************************************************************************************/
    property UiSession  uiSession
    property I_Panel    currentPanel: null

    /* Object Properties
     * ****************************************************************************************/
    color: "#aa000000"
    visible: false

    /* Functions
     * ****************************************************************************************/
    //! Shows the provided pannel and hide any others
    function displayPanel(panel) {
        // Hide the current panel, if any
        if (currentPanel !== null) {
            currentPanel.visible = false;
        }

        // Show the new panel
        currentPanel                    = panel;
        currentPanel.parent             = panelLayout;
        currentPanel.anchors.centerIn   = panelLayout;
        currentPanel.visible            = true;

        panelLayout.visible  = true;
    }

    //! Closes the provided panel and shows the one on top of the stack
    function closePanel(panel) {
        // Hide the current panel
        currentPanel.visible            = false;
        currentPanel                    = null;

        // Show any pending panels
        if (uiSession.panelStack.length !== 0) {
            displayPanel(uiSession.panelStack[uiSession.panelStack.length - 1]);
        }
        // Otherwise hide the layout
        else {
            panelLayout.visible = false;
        }
    }

    /* Connections
     * ****************************************************************************************/
    Connections {
        target: uiSession
        function onSigShowPanel(panel) {
            panelLayout.displayPanel(panel);
        }
    }

    Connections {
        target: uiSession
        function onSigHidePanel(panel) {
            panelLayout.closePanel(panel);
        }
    }

    /* Children
     * ****************************************************************************************/
    // Sink for all mouse events outside the panel
    MouseArea {
        anchors.fill: parent
        enabled: currentPanel !== null && currentPanel.visible
    }
}
