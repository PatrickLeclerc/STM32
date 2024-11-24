#ifndef APP_HPP
#define APP_HPP
#include "drivers_common.hpp"

class App
{
public:
    App(){};
    ~App(){deinit();};
    void init(){};
    void deinit(){};
};

#endif
