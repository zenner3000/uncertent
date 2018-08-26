#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#endif // MISC_H_INCLUDED

#include <string>
#include <map>
using namespace std;

string sha256_then_base64(const string &data, const string &key);
string getutctime();
string parmmap_to_parmstr(map<string,string> mparm);
string gettimestamp() ;
string b2a_hex(char *byte_arr, int n) ;
string sha256_then_str(string data, string key);
string md5_then_str(const string &data);
string& to_lower(string& str);
string& to_upper(string& str);
string to_lowercase(string str);
string to_uppercase(string str);

