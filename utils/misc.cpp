

#include "misc.h"

#include "time.h"
#include "openssl/sha.h"
#include "openssl/hmac.h"
#include "openssl/md5.h"
#include <string>
#include <sstream>
#include <map>
#include "codec.h"
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cmath>
using namespace std;


string b2a_hex(char *byte_arr, int n) {

	const static std::string HexCodes = "0123456789abcdef";
	string HexString;
	for (int i = 0; i < n; ++i) {
		unsigned char BinValue = byte_arr[i];
		HexString += HexCodes[(BinValue >> 4) & 0x0F];
		HexString += HexCodes[BinValue & 0x0F];
	}
	return HexString;
}

string sha256_then_base64(const string &data, const string &key) {

	unsigned char * result;
	unsigned int len = 32;
	result = new unsigned char[len];
	string retstr;

	HMAC_CTX *ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, key.c_str(), key.length(), EVP_sha256(), NULL);
	HMAC_Update(ctx, (unsigned char*)(data.c_str()), data.length());
	HMAC_Final(ctx, result, &len);

	retstr = base64_encode(result, len);
	delete []result;
	return retstr;
}

string sha256_then_str(string data, string key) {

	unsigned char * result;
	unsigned int len = 32;
	result = new unsigned char[len];
	string retstr;

	HMAC_CTX *ctx = HMAC_CTX_new();
	HMAC_Init_ex(ctx, key.c_str(), key.length(), EVP_sha256(), NULL);
	HMAC_Update(ctx, (unsigned char*)(data.c_str()), data.length());
	HMAC_Final(ctx, result, &len);

	retstr = b2a_hex((char*)result, len);

	delete[]result;
	return retstr;
}

string md5_then_str(const string &data){
    string retstr;
    MD5_CTX ctx;
    unsigned char  digest[16] = {0};
    MD5_Init(&ctx);
    MD5_Update(&ctx,data.c_str(),data.length());
    MD5_Final(digest,&ctx);
    stringstream ss;
    for(int i=0;i!=16;i++){
        ss<<std::setfill('0')<<std::setw(2)<<std::uppercase;
        ss<<hex<<(unsigned int)digest[i];
    }
    return ss.str();
}

string getutctime() {
	time_t timep = {0};
	time(&timep);
	char tmp[64] = {0};
	string rettime;
	struct tm *timeinfo = gmtime(&timep);
	strftime(tmp, sizeof(tmp), "%Y-%m-%dT%H:%M:%S", timeinfo);
	rettime.assign(tmp);
	//cout << rettime << endl;
	return rettime;
}


string gettimestamp() {
	using namespace std::chrono;
	milliseconds ms = duration_cast< milliseconds >(
		system_clock::now().time_since_epoch()
		);
	return to_string(ms.count());
}



string parmmap_to_parmstr(map<string,string> mparm){
    string retstr;
    map<string,string>::iterator it;

    for(it=mparm.begin();it!=mparm.end();it++){
        retstr.append(it->first);
        retstr.append("=");
        retstr.append(it->second);
        retstr.append("&");
    }
    //delete the last character
    retstr.erase(retstr.length()-1);
    return retstr;
}

string& to_lower(string& str)
{
    transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(tolower));
    return str;
}

string& to_upper(string& str)
{
    transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(toupper));
    return str;
}

string to_lowercase(string str)
{
    transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(tolower));
    return str;
}

string to_uppercase(string str)
{
    transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(toupper));
    return str;
}

