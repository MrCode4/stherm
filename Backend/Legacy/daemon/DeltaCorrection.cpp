#include "DeltaCorrection.h"
#include "math.h"




DeltaCorrection::DeltaCorrection()
{
  
}

double DeltaCorrection::getDeltaTemp() const
{
    return DeltaTemp;
}


void DeltaCorrection::CalculateDeltaTemp()
{
    if (state == HEATING_PROCCES)
    {
        if (current_time - time_ON < 0)
            std::cout << "Error with class integration";
        else if (current_time - time_ON < 30)
            DeltaTemp = delta_heating(current_time - time_ON, Backlight_factor);
        else
        {
            state = WARMED;
            DeltaTemp = 3.1*Backlight_factor;
        }
        return;
    }
    if (state == COOLING_PROCCES)
    {
        if (current_time - time_OFF < 0)
            std::cout << "Error with class integration";
        else if (current_time - time_OFF < 13)
            DeltaTemp = delta_cooling(current_time - time_OFF);
        else
        {
            DeltaTemp = 0.5;
            state = FROZEN;
        }
         return;
    }
    if (state == WARMED)
    {
        DeltaTemp = 3.1*Backlight_factor;
        return;
    }

    DeltaTemp = 0.5;
  

}
 double  DeltaCorrection::delta_cooling(double time)
 {
         return(0.5+ (3.34*exp(-0.2*time) -0.24));  //K ?
 }

double DeltaCorrection::delta_heating(double  time, double Backlight_factor)
{
    return (0.5 + Backlight_factor * pow(time, (1.0 / 3)));
}


double DeltaCorrection::cooling_interval_in_heating(double heating_time)
{
    double T_1 = pow(heating_time, (1.0 / 3));
    double cooling_time = 5 * log(167 / (2 * 25 * T_1 + 6));
    return(cooling_time);
}

double DeltaCorrection::heating_interval_in_cooling(double cooling_time)
{
        double T_1 = 3.34 * exp(-0.2 * cooling_time) - 0.24;
        double heating_time = pow(T_1, 3);
        return(heating_time);
}

void        DeltaCorrection::update_ON(int time, double Backlight_factor)
{

        double   current_on_time = static_cast<double>(time) / 60;
        this->Backlight_factor = Backlight_factor;
        if (state == COOLING_PROCCES)
        {
            time_ON = current_on_time - heating_interval_in_cooling(current_on_time - time_OFF);
        }
        else if (state == DEFAULT || state == FROZEN)
        {
            time_ON = current_on_time;
        }
        state = HEATING_PROCCES;
        
}

void        DeltaCorrection::update_OFF(int time)
{
    double   current_off_time = static_cast<double>(time) / 60;
    if (state == WARMED)
        time_OFF = current_off_time;
    else if (state == HEATING_PROCCES)
        time_OFF = current_off_time - cooling_interval_in_heating(current_off_time - time_ON);
    state = COOLING_PROCCES;
    
}

void        DeltaCorrection::update_Time(int time)
{
            current_time = static_cast<double>(time) / 60;
            if ((state == HEATING_PROCCES) && (current_time - time_ON) > 30)
            {
                state = WARMED;
            }
         
            if ((state == COOLING_PROCCES) && (current_time - time_OFF) > 13)
            {
                state = FROZEN;
            }
            CalculateDeltaTemp();
}
