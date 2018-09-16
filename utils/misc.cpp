

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
#include <stdio.h>

#include "zlib.h"
#define SET_BINARY_MODE(file)
#define CHUNK 16384
#define windowBits 15
#define GZIP_ENCODING 16

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


bool gzip_compress(const std::string& data, std::string& compressedData, int level)
{
  unsigned char out[CHUNK];
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  if (deflateInit2(&strm, level, Z_DEFLATED, windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK)
  {
    return false;
  }
  strm.next_in = (unsigned char*)data.c_str();
  strm.avail_in = data.size();
  do {
    int have;
    strm.avail_out = CHUNK;
    strm.next_out = out;
    if (deflate(&strm, Z_FINISH) == Z_STREAM_ERROR)
    {
      return false;
    }
    have = CHUNK - strm.avail_out;
    compressedData.append((char*)out, have);
  } while (strm.avail_out == 0);
  if (deflateEnd(&strm) != Z_OK)
  {
    return false;
  }
  return true;
}

bool gzip_uncompress(const string& compressedData, string& data)
{
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char out[CHUNK];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
  {
    return false;
  }

  strm.avail_in = compressedData.size();
  strm.next_in = (unsigned char*)compressedData.c_str();
  do {
    strm.avail_out = CHUNK;
    strm.next_out = out;
    ret = inflate(&strm, Z_NO_FLUSH);
    switch (ret) {
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      inflateEnd(&strm);
      return false;
    }
    have = CHUNK - strm.avail_out;
    data.append((char*)out, have);
  } while (strm.avail_out == 0);

  if (inflateEnd(&strm) != Z_OK) {
    return false;
  }

  return true;
}

//zlib format (deflate format)
int deflatetest(char * src, int srclen, string& data)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    strm.avail_in = srclen;
    strm.next_in = (unsigned char*)src;

    /* run inflate() on input until output buffer not full */
    do {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);
        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     /* and fall through */
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return ret;
        }
        have = CHUNK - strm.avail_out;
        data.append((char*)out, have);
    } while (strm.avail_out == 0);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

//zlib format (deflate format)
bool inflate_uncompress(const string& compressedData, string& data)
{
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char out[CHUNK];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  if (inflateInit2(&strm, -MAX_WBITS) != Z_OK)   // import!!!!!
  {
    return false;
  }

  strm.avail_in = compressedData.size();
  strm.next_in = (unsigned char*)compressedData.c_str();
  do {
    strm.avail_out = CHUNK;
    strm.next_out = out;
    ret = inflate(&strm, Z_NO_FLUSH);
    switch (ret) {
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      inflateEnd(&strm);
      return false;
    }
    have = CHUNK - strm.avail_out;
    data.append((char*)out, have);
  } while (strm.avail_out == 0);

  if (inflateEnd(&strm) != Z_OK) {
    return false;
  }

  return true;
}
