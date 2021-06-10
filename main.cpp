#include "main.h"
#include "sql.h"

int code_convert(char *inbuf, int inlen, char *outbuf, int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open("UTF-8", "GB2312");
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}

//解析post请求数据
void get_post_message(std::string &buf, struct evhttp_request *req)
{
    size_t post_size = 0;

    post_size = evbuffer_get_length(req->input_buffer); //获取数据长度
    //std::cout << "====line:" << __LINE__ << " len:" << post_size << std::endl;
    if (post_size <= 0)
    {
        //std::cout << "====line:" << __LINE__ << std::endl;
        return;
    }
    else
    {
        buf = std::string((const char *)evbuffer_pullup(req->input_buffer, -1));
    }
}
//new
void login(evhttp_request *req, void *arg)
{
    if (req == nullptr)
    {
        //std::cout << "====line:" << __LINE__ << "input param req is null." << std::endl;
        return;
    }
    std::string buf;
    get_post_message(buf, req); //获取请求数据，一般是json格式的数据
    if (buf.empty())
    {
        //std::cout << "====line:" << __LINE__ << "get_post_message return null." << std::endl;
        return;
    }

    //可以使用json库解析需要的数据

    //响应的buffer
    struct evbuffer *retbuff = evbuffer_new();
    cJSON *monitor = cJSON_CreateObject();

    //请求数据格式错误
    cJSON *json = cJSON_Parse(buf.c_str());
    if (!json)
    {
        cJSON_AddItemToObject(monitor, "data", nullptr);
        cJSON_AddStringToObject(monitor, "msg", "数据格式错误");
        cJSON_AddNullToObject(monitor, "systemCode");
        cJSON_AddNumberToObject(monitor, "statusCode", 400);
        std::string tmp(cJSON_Print(monitor));
        evbuffer_add_printf(retbuff, "%s", tmp.c_str());
        evhttp_send_reply(req, HTTP_BADREQUEST, "bad request", retbuff);
        evbuffer_free(retbuff);
        return;
    }

    std::vector<std::string> item = {"", "Username", "Password", "Phone", "Sex",
                                     "Email", "Integral", "Birthday",
                                     "Membership_code", "Create_time", "Update_time"};

    cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
    cJSON *password = cJSON_GetObjectItemCaseSensitive(json, "password");
    std::string sql = "select * from user where Username = \"" + std::string(username->valuestring) +
                      "\" and Password = \"" + std::string(password->valuestring) + "\";";
    //std::cout << sql << std::endl;
    MYSQL_RES *res = MyDB::getInstance()->searchValuesToRes(sql.c_str());
    //查询结果正确
    if (res) // 返回了结果集
    {
        cJSON *monitor2 = cJSON_CreateObject();
        int num_fields = mysql_num_fields(res); //获取结果集中总共的字段数，即列数
        //获取下一行数据
        MYSQL_ROW row = mysql_fetch_row(res);
        for (int j = 1; j < num_fields; j++) //输出每一字段
        {
            std::cout << row[j] << "   ";
            cJSON_AddStringToObject(monitor2, item[j].c_str(), row[j]);
        }
        std::cout << std::endl;
        cJSON_AddItemToObject(monitor, "data", monitor2);
        cJSON_AddStringToObject(monitor, "msg", "login success");
        cJSON_AddNullToObject(monitor, "systemCode");
        cJSON_AddNumberToObject(monitor, "statusCode", 200);
        std::string tmp(cJSON_Print(monitor));
        evbuffer_add_printf(retbuff, "%s", tmp.c_str());
        evhttp_send_reply(req, HTTP_OK, "", retbuff);
        evbuffer_free(retbuff);
        cJSON_free(monitor2);
    }
    //查询结果错误
    else
    {
        cJSON_AddItemToObject(monitor, "data", nullptr);
        cJSON_AddStringToObject(monitor, "msg", "查询结果为空");
        cJSON_AddNullToObject(monitor, "systemCode");
        cJSON_AddNumberToObject(monitor, "statusCode", 200);
        std::string tmp(cJSON_Print(monitor));
        evbuffer_add_printf(retbuff, "%s", tmp.c_str());
        evhttp_send_reply(req, HTTP_BADREQUEST, "", retbuff);
        evbuffer_free(retbuff);
    }
    cJSON_free(monitor);
    cJSON_free(username);
    cJSON_free(password);
}

void page404(evhttp_request *req, void *arg)
{
    struct evbuffer *retbuff = nullptr;
    retbuff = evbuffer_new();
    if (!retbuff)
    {
        std::cerr << "====line:" << __LINE__ << "retbuff in nullptr" << std::endl;
        return;
    }
    evbuffer_add_printf(retbuff, "404 NOT FOUND!");
    evhttp_send_reply(req, HTTP_NOTFOUND, "no resource", retbuff);
    evbuffer_free(retbuff);
}

int main()
{
    MyDB::getInstance()->connect("10.3.35.247", "root", "rootrootroot", "heytea");
    struct evhttp *http_server = nullptr;
    //初始化
    event_init();
    //启动http服务端
    http_server = evhttp_start(http_addr.c_str(), http_port);
    if (http_server == nullptr)
    {
        std::cerr << "http_server start failed!" << std::endl;
        return -1;
    }
    //设置请求超时时间(s)
    evhttp_set_timeout(http_server, 5);
    //设置事件处理函数，evhttp_set_cb针对每一个事件(请求)注册一个处理函数，
    //区别于evhttp_set_gencb函数，是对所有请求设置一个统一的处理函数
    evhttp_set_gencb(http_server, page404, nullptr);
    evhttp_set_cb(http_server, "/api/login/login", login, nullptr);

    //循环监听
    event_dispatch();
    //实际上不会释放，代码不会运行到这一步
    evhttp_free(http_server);

    return 0;
}