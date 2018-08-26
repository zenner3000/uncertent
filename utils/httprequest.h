#ifndef HTTPREQUEST_INCLUDED
#define HTTPREQUEST_INCLUDED

#endif // HTTPREQUEST_INCLUDED

#include <vector>
using namespace std;
/*
url , domain + uri + parm
header ,  http header you need to append
*/
string https_get(const string &url,const vector<string> &headerlist);
string https_post(const string &url,const string &data, const vector<string> &headerlist);
