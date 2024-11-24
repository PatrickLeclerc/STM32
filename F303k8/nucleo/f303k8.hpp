#include "clock.hpp"
#include "led.hpp"
#include "console.hpp"

#include <vector>

class F303k8
{
private:
    Clock clock;
public:
    // Constructors / Destructors
    F303k8();
    ~F303k8();
    // Apps
    Console console;
};

