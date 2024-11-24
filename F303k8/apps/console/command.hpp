#include <string>
#include <functional>
#include <stdint.h>

class Command
{
public:
    Command(std::string name, std::string help, std::function<void()> func) : name(name), help(help), func(func), argc(0){};
    Command(std::string name, std::string help, std::function<void()> func, int8_t n) : name(name), help(help), func(func), argc(n){};
    std::string name;
    int8_t argc;
    std::string help; 
    std::function<void()> func;
};