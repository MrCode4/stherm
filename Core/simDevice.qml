import QtQuick
import Stherm

/*! ***********************************************************************************************
 * Simulation Device
 * ************************************************************************************************/
I_Device {
    id: simDevice

    /* Property Declarations
     * ****************************************************************************************/
    property SimDeviceController _controller: SimDeviceController {
        device: simDevice
    }


    /* Object Properties
     * ****************************************************************************************/
    type: AppSpec.DeviceType.DT_Sim
}
