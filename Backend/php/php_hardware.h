#ifndef PHP_HARDWARE_H
#define PHP_HARDWARE_H

#include <QObject>
#include <QQmlEngine>

class php_hardware : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_hardware(QObject *parent = nullptr);

    /**
     * @brief Get the Start Mode
     * return an integer that defines if the device is started in test or normal mode,
     * or if this is the first time the device has been run
     * 
     * TODO split this functionality into isTestMode, isFirstRunMode, with normal mode implied if neither other are positive
     * 
     * @return int 0=test mode, 1=normal mode, 2=first run mode
     */
    int getStartMode(void);


    // TODO move backlight to its own class?

    enum BacklightMode { on, slowBlink, fastBlink };

    /// @brief the state of the/a backlight
    struct Backlight
    {
        int             red;
        int             green;
        int             blue;
        BacklightMode   type;
        bool            onOff;
    };
    

    /**
     * @brief Control the backlight via nRF co-processor
     * 
     * TODO type should be more self explantory, e.g. bool blink, int blinkRate
     * TODO add a method or overload to turn off
     * 
     * @param red       red level, 0-255
     * @param green     green level, 0-255
     * @param blue      blue level, 0-255
     * @param type      0=on, 1=slow blink, 2=fast blink
     * @param onOff     true=on, false=off
     * @return true     success
     * @return false    failure
     */
    bool setBacklight(int red, int green, int blue, int type, bool onOff);

    /**
     * @brief Control the backlight via nRF co-processor
     * 
     * @param backlight backligh structure definition
     * @return true     success
     * @return false    failure
     */
    bool setBacklight(Backlight backlight);

    /**
     * @brief Return the current state of the backlight
     * 
     * TODO if variables are managed inside the UI, we shoudl be able to reference the Backlight variable
     * 
     * @return Backlight 
     */
    Backlight getBacklight(void);



    struct Wiring
    {
        bool    R;
        bool    C;
        bool    G;
        bool    Y1;
        bool    Y2;
        bool    ACC2;
        bool    ACC1P;
        bool    ACC1N;
        bool    W1;
        bool    W2;
        bool    W3;
        bool    OB;
    };

    // TBC, returns an array for 12 paramaters, one for each wiring
    // [R,C,G,Y1,Y2,ACC2,ACC1P,ACC1N,W1,W2,W3,OB]
    // The difference between this and below, is a bunch of data manipulations and database upadtes
    Wiring getWiring(void);
    
    /**
     * @brief Get actual wiring from the database without requesting
     * 
     * TODO this naming needs refactoring, as the php description suggests this is the database version, and NOT the actual wiring data
     * 
     * @return Wiring 
     */
    Wiring getActualWiring(void);


    /**
     * @brief Check if the device has a client
     * 
     * TODO despite the name this function will actually update the remote database via sync changContractorInfo
     * 
     * 
     * @return true 
     * @return false 
     */
    bool checkClient(void);


    /// @brief device configuration settings
    struct Settings
    {
        int     brightness;
        int     speaker;
        int     measure_id;
        int     time;
        bool    reset;
        bool    adaptive_brightness;    //!< true = adaptive, false = manual
        int     system_delay;
        int     measure;
    };
    

    Settings getSettings(void);

    bool setSettings(Settings);


    /**
     * @brief Set the Brightness value and mode
     * 
     * calls the setBrightness executable.. TBC
     * 
     * TODO brightness value and adaptive settings could just be getter/setter
     * 
     * @param brightness    Brightness value
     * @param adaptive      true = adaptive, false = manual
     * @return true         success
     * @return false        failure
     */
    bool setBrightness(int brightness, bool adaptive);


signals:

};

#endif // PHP_HARDWARE_H
