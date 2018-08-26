#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

#endif // TEST_H_INCLUDED


#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <map>
#include <time.h>
//#include "cppcodec/cppcodec/base64_default_rfc4648.hpp"

using namespace std;


/*
string genuri_byparm(const map<string ,string> &mparm){
     string head = "GET\n";
     head.append("api.huobi.pro\n");
     head.append("/v1/account/accounts\n");
     string seckey = "ae135856-91dbb36a-cee4149f-8e037";

     string uri = parmmap_to_parmstr(mparm);
     uri.erase(uri.length()-1);
     cout<< "head--"<<head.append(uri) <<endl;
     string signedstr = sha256_then_base64(head,seckey);
     return uri + "&" + "Signature=" + urlEncode(signedstr);
}
*/

/*
every method
1 parm list  --> map parm
2 make parm string by map , (using & connect)
3 sign = sha256base64
4 domain + parmlist  + & + sign =value
5 http request

class
sign
httprequest(string url)
create order (cointain ,/xxx/v1/vyyy)
cancel order
get open orders
get specific order detail
fetch symbols
get balance
domain ,seckey,acckey
*/




