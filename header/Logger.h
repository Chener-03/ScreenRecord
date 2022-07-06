//
// Created by chen on 2022/6/30.
//

#ifndef UNTITLED_LOGGER_H
#define UNTITLED_LOGGER_H
#include "iostream"

#define INFOOUT(ch,...) printf("%s %s %s(%s) [INFO] "#ch"\n",__DATE__,__TIME__,__FILE__ , __func__,##__VA_ARGS__)
#define ERROROUT(ch,...) printf("%s %s %s(%s:%d) [ERROR] "#ch"\n",__DATE__,__TIME__,__FILE__ , __func__,__LINE__,#__VA_ARGS__)


#endif //UNTITLED_LOGGER_H
