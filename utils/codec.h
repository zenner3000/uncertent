#ifndef CODEC_H_INCLUDED
#define CODEC_H_INCLUDED

#include <string>

using namespace std;

string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) ;
string base64_decode(string const& encoded_string) ;


/*encode the string*/
string base64_encodestr(const string &in) ;
/*decode the string*/
string base64_decodestr(const string &in) ;

/*encode url*/
string urlEncode(string str) ;

char dec2hexChar(short int n) ;
short int hexChar2dec(char c) ;

string escapeURL(const string &URL);
string deescapeURL(const string &URL) ;


#endif // CODEC_H_INCLUDED

