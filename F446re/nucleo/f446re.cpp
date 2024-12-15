#include "f446re.hpp"
F446re::F446re(){
    led.init();
    console.init();
    sd.init();
}
F446re::~F446re(){}
