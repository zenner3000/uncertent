#ifndef HTTPREQUEST_INCLUDED
#define HTTPREQUEST_INCLUDED

#include <vector>

/*
url , domain + uri + parm
header ,  http header you need to append
*/
string https_get(const string &url,const std::vector<string> &headerlist);
string https_post(const string &url,const string &data, const std::vector<string> &headerlist);

#endif // HTTPREQUEST_INCLUDED
