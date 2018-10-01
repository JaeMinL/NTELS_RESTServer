#include <curl/curl.h>
#include <stdlib.h>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{

    sprintf(stream, "%s",ptr);

    return nmemb * size;
}

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    /* 'userdata' is set with CURLOPT_HEADERDATA */
    fprintf(stderr,"HDR: %s", buffer);

    return nitems * size;
}

int main()
{
    CURL *hnd = curl_easy_init();
    char dat[1024];

    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_URL, "http://192.168.62.17:8080/v1/statistics/resource/cpu?start-time=1508811860&end-time=1513004400&node-id=ATOM");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "content-type: application/json");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(hnd, CURLOPT_HEADER, 1);

    /* setting post data */
    //curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "{\n\t\"USER\": \"abc\",\n\t\"PASSWORD\": \"1\"\n}");

    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, dat);
    curl_easy_setopt(hnd,  CURLOPT_WRITEFUNCTION, write_data);

    curl_easy_setopt(hnd, CURLOPT_HEADERFUNCTION, header_callback);

    CURLcode ret = curl_easy_perform(hnd);

    fprintf(stderr,"DATA = [%s]\n",dat);

    /* get response code */
    long response_code;
    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &response_code);

    printf("RESPONSE CODE = %d\n",response_code);
}

