#include <string>
#include <functional>
#include <stdint.h>

class Command
{
public:
    Command(std::string name, std::string help, std::function<void()> func              ) : name(name), help(help), func(func), argc(0){};
    Command(std::string name, std::string help, std::function<void()> func, int argc) : name(name), help(help), func(func), argc(argc){};
    std::string name;
    int argc;
    std::string help; 
    std::function<void()> func;
};