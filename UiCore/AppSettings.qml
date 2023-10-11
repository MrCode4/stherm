pragma Singleton

import QtQuick
import QTemplateProject

/*! ***********************************************************************************************
 * This class extends AppSettingsCPP and is responsible for saving/loading
 * general properties in the OS. the underlying layer is using QSettings
 * for this purpose so it's cross platform.
 * In windows we are using registery for saving global settings
 * In other os, QSettings handles where to store the properties.
 * ************************************************************************************************/
AppSettingsCPP {
    /* Property Declarations
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/

}
