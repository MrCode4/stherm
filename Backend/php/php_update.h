#ifndef PHP_UPDATE_H
#define PHP_UPDATE_H

#include <QObject>
#include <QQmlEngine>

class php_update : public QObject
{
    Q_OBJECT
    //    QML_ELEMENT
public:
    explicit php_update(QObject *parent = nullptr);


    /**
     * Fetches software update information.
     * 
     * Retrieves the current software version from a configuration file, 
     * constructs a request to a web service to check for available updates, 
     * and processes the response. The method provides details on the current 
     * version, UID, update timestamp, and the availability of a new version.
     * 
     * @return array An array containing the current software version, UID, 
     *               update timestamp, version details, and whether an update is available.
     */
    void getSoftwareUpdateInfo(void);

    /**
     * Initiates the installation of a new software version.
     * 
     * Uses the device's serial number to fetch and install a software update.
     * 
     * @return false|string|null The result of the update action, which might be 
     *                           a success message, error, or another status.
     * @throws Exception Throws an exception if the update process encounters an issue.
     */
    void install(void);

    /**
     * Requests and processes system updates based on the provided device serial number.
     *
     * The function fetches the current software version from a configuration file, 
     * and then makes a request to an external service to check if there are any system updates 
     * available for the device with the provided serial number.
     *
     * If an update is available, the function either processes a full system update or a partial 
     * update based on the response from the external service.
     *
     * For a full system update, it:
     * - Clears specific directories.
     * - Retrieves WiFi connection details and stores them.
     * - Downloads the update files based on the provided URLs.
     * - Optionally, there's logic for checksum validation which seems to be commented out.
     *
     * For a partial update, it:
     * - Downloads the update ZIP file.
     * - Extracts the ZIP content.
     * - Executes the update script.
     *
     * @param string $serial_number The serial number of the device to be updated.
     * @return mixed Returns the output of the update process or `false` if an error occurs.
     */
    void getUpdate(void);


signals:

};

#endif // PHP_UPDATE_H
