#include "clock.hpp"
#include "led.hpp"
#include "console.hpp"
#include "sdio.hpp"
#include <vector>

class F446re
{
private:
    Clock clock;
public:
    // Constructors / Destructors
    F446re();
    ~F446re();
    // Apps
    LED  led;
    Console console;
};

