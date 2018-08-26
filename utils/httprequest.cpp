#ifndef HTTPREQUEST_INCLUDED
#define HTTPREQUEST_INCLUDED

#endif // HTTPREQUEST_INCLUDED

#include <curl/curl.h>
#include <string>
#include <iostream>
#include <vector>


using namespace std;


static int writer_get_callback(char *data, size_t size, size_t nmemb,std::string *writerData)
{
  if(writerData == NULL)
    return 0;

  writerData->append(data, size*nmemb);

  return size * nmemb;
}


/*
url , domain + uri + parm
header ,  http header you need to append
*/
string https_get(const string &url,const vector<string> &headerlist){
    //cout <<url <<endl;
    CURL *curl;
    CURLcode res;
    string retstr ;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    struct curl_slist *headers =  NULL;
    vector<string>::const_iterator it;
    for (it=headerlist.begin(); it != headerlist.end(); it++){
        headers = curl_slist_append(headers,it->c_str());
        //cout << *it <<endl;
    }
    curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer_get_callback);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&retstr);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    //sleep(5);
    //cout <<"+++"<<retstr << endl;

    //handle response


    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));



    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

    curl_global_cleanup();
    return retstr;
}


static int writer_post_callback(char *data, size_t size, size_t nmemb,std::string *writerData)
{
  if(writerData == NULL)
    return 0;

  writerData->append(data, size*nmemb);
  return size * nmemb;
}



string https_post(const string &url, const string &data, const vector<string> &headerlist){
    //cout <<url <<endl;
    //cout <<data <<endl;
    CURL *curl;
    CURLcode res;
    string retstr ;

    //curl_global_init(CURL_GLOBAL_ALL);
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer_post_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&retstr);


    struct curl_slist *headers =  NULL;
    vector<string>::const_iterator it;
    for (it=headerlist.begin(); it != headerlist.end(); it++){
        headers = curl_slist_append(headers,it->c_str());
        //cout << *it <<endl;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,headers);

    //curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    //sleep(5);
    //cout <<"retstr-----"<<retstr << endl;

    //handle response

    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

    curl_global_cleanup();
    return retstr;
}
