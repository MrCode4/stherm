#include "php_hardware.h"

php_hardware::php_hardware(QObject *parent)
    : QObject{parent}
{

}

int php_hardware::getStartMode(void)
{
    return 0;
}

bool php_hardware::setBacklight(int red, int green, int blue, int type, bool onOff)
{
    return false;
}

bool php_hardware::setBacklight(Backlight backlight)
{
    return false;
}
