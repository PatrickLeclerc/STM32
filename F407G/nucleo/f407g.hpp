#include "clock.hpp"
#include "led.hpp"
#include "console.hpp"

class F407G
{
private:
    Clock clock;
public:
    // Constructors / Destructors
    F407G();
    ~F407G();
    // Apps
    LED led;
    Console console;
};
