#include "http/d_http.h"
#include "log.h"


static dreamer::Logger::ptr g_logger = DREAMER_SYSTEM_LOGGER();
void testHttpRequest() {
    dreamer::http::HttpRequest httpRequest;
    httpRequest.setHeader("host" , "www.baidu.com");
    httpRequest.setBody("hello Sylar");
    httpRequest.setUri("https://www.baidu.com/s?wd=vim%20%E6%9B%BF%E6%8D%A2%E5%AD%97%E7%AC%A6&rsv_spt=1&rsv_iqid=0x88db190d00074ee1&issp=1&f=8&rsv_bp=1&rsv_idx=2&ie=utf-8&rqlang=cn&tn=baiduhome_pg&rsv_enter=1&rsv_dl=tb&rsv_btype=t&inputT=1661&rsv_t=f593wPaTYs2e41wSVfrzzGf%2F2a2q0JK3J0CuepMMxoStLzbqtNVMDQKkTirWj%2BKoaLPT&oq=vim%2520%25E5%25A6%2582%25E4%25BD%2595%25E8%25B7%25B3%25E8%25BD%25AC%25E5%25A3%25B0%25E6%2598%258E&rsv_pq=9cba9f4b0000e28f&rsv_sug2=0&rsv_sug4=1928");
    httpRequest.setMethod(dreamer::http::HttpMethod::GET);
    D_SLOG_INFO(g_logger) << httpRequest.toString();
    
}


int main() {
    testHttpRequest();
}