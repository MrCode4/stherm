#pragma once

#define DEFAULT_STATE -1
#include <iostream>
typedef enum PROCCESS
{
    HEATING_PROCCES,
    COOLING_PROCCES,
    FROZEN,
    WARMED,
    DEFAULT


} M_PROCCESS;

class DeltaCorrection
{
    public:
                        DeltaCorrection();

        double          getDeltaTemp() const;
        void            update_ON(int time, double Backlight_factor);
        void            update_OFF(int time);
        void            update_Time(int time);
    private:

    //    int         counter;
        void        CalculateDeltaTemp();
        int         current_time;
        double      DeltaTemp = 0.5;
        
        double      delta_heating(double  time, double Backlight_factor);
        double      delta_cooling(double  time);

        double      Backlight_factor = 1;
        int         time_ON = 0;
        int         time_OFF = 0;
        PROCCESS    state = DEFAULT;
       

        double cooling_interval_in_heating(double heating_time);
        double heating_interval_in_cooling(double cooling_time);


};


