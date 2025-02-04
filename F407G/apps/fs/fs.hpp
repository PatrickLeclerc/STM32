#ifndef FS_HPP
#define FS_HPP

#include "app.hpp"

#include "sdio.hpp"
#include "ff.h"
#include "diskio.h"

#include <string>

extern SD _sd;

class FS : public App
{
public:
    FATFS fatfs;
    std::string logs = "";
    void init();
    void deinit(){};
};

#endif
