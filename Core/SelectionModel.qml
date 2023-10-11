import QtQuick
import QTemplateProject
import QtQuickStream

/*! ***********************************************************************************************
 * Selection model is responsible to keep track of selected
 * control and provides appropriate API to select/deselect
 * controls
 * ************************************************************************************************/
QSObject {
    /* Property Declarations
     * ****************************************************************************************/


    //! Dashboard UUID
    property string selectedDashboard:    ""

    //! Device UUID
    property string selectedDevice:       ""

    //! Plot UUID
    property string selectedPlot:         ""

    /* Signals
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/
    function clear() {
    }

    function clearDashboardSelection() {
        selectedDashboard = null
    }
    
    function clearDeviceSelection() {
        selectedDevice = null
    }

    function clearPlotSelection() {
        selectedPlot = null
    }

    function reset() {

    }

    function selectDashboard(dashboard: string) {
        selectedDashboard = dashboard;
    }

    function selectDevice(device: string) {
        selectedDevice = device;
    }

    function selectPlot(plot: string) {
        selectedPlot = plot;
    }
}
