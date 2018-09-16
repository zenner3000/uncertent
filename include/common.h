#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <map>
#include "zlib.h"
using namespace std;

struct askbidtable{
    map<double,double> ask_table;
    map<double,double> bid_table;
};




#endif // COMMON_H_INCLUDED
