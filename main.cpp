#include "main.h"
#include "sql.h"

//解析post请求数据
void get_post_message(std::string &buf, struct evhttp_request *req)
{
    size_t post_size = 0;

    post_size = evbuffer_get_length(req->input_buffer); //获取数据长度
    std::cout << "====line:" << __LINE__ << " len:" << post_size << std::endl;
    if (post_size <= 0)
    {
        std::cout << "====line:" << __LINE__ << std::endl;
        return;
    }
    else
    {
        //size_t copy_len = post_size > BUF_MAX ? BUF_MAX : post_size;
        //printf("====line:%d,post len:%ld, copy_len:%ld\n", __LINE__, post_size, copy_len);
        //memcpy(buf, evbuffer_pullup(req->input_buffer, -1), copy_len);
        buf = std::string((const char *)evbuffer_pullup(req->input_buffer, -1));
        buf[post_size] = '\0';
        //printf("====line:%d,post msg:%s\n", __LINE__, buf);
    }
}

void http_handler_post(struct evhttp_request *req, void *arg)
{
    std::cout << "http_handler_post" << std::endl;
}
void http_handler_get(struct evhttp_request *req, void *arg)
{
    std::cout << "http_handler_get" << std::endl;
}

void http_handler_index(struct evhttp_request *req, void *arg)
{
    std::cout << "http_handler_reqType" << std::endl;

    //查看请求类型
    evhttp_cmd_type type = evhttp_request_get_command(req);

    switch (type)
    {
    case EVHTTP_REQ_GET:

        break;
    case EVHTTP_REQ_POST:

        break;
    default:
        break;
    }
}

void login(evhttp_request *req, void *arg)
{
    const cJSON *name = nullptr;
    const cJSON *pass = nullptr;
    cJSON *monitor = cJSON_CreateObject();
    if (req == nullptr)
    {
        std::cout << "====line:" << __LINE__ << "input param req is null." << std::endl;
        return;
    }

    std::string buf(BUF_MAX, 0);
    get_post_message(buf, req); //获取请求数据，一般是json格式的数据
    if (buf.empty())
    {
        std::cout << "====line:" << __LINE__ << "get_post_message return null." << std::endl;
        return;
    }
    else
    {
        //可以使用json库解析需要的数据

        cJSON *json = cJSON_Parse(buf.c_str());
        if (!json)
        {
            struct evbuffer *retbuff = evbuffer_new();
            evbuffer_add_printf(retbuff, "数据格式错误!");
            evhttp_send_reply(req, HTTP_BADREQUEST, "bad request", retbuff);
            evbuffer_free(retbuff);
            return;
        }

        std::vector<std::string> item = {"Username", "Password", "Phone", "Sex", "Email", "Itegral", "Birthday", "Membership_code", "Create_time", "Update_time"};
        name = cJSON_GetObjectItemCaseSensitive(json, "username");
        pass = cJSON_GetObjectItemCaseSensitive(json, "password");

        std::string sql = "select * from user where Username = \"" + std::string(name->valuestring) +
                          "\" and Password = \"" + std::string(pass->valuestring) + "\";";
        std::cout << sql << std::endl;
        MYSQL_RES *res = MyDB::getInstance()->searchValuesToRes(sql.c_str());
        if (res) // 返回了结果集
        {
            cJSON *monitor2 = cJSON_CreateObject();
            int num_fields = mysql_num_fields(res); //获取结果集中总共的字段数，即列数
            //int num_rows = mysql_num_rows(res);     //获取结果集中总共的行数
            MYSQL_ROW row;
            // for (int i = 0; i < num_rows; i++) //输出每一行
            // {
            //获取下一行数据
            row = mysql_fetch_row(res);
            // if (row < 0)
            //     break;

            for (int j = 0; j < num_fields; j++) //输出每一字段
            {
                //std::cout << row[j] << "\t\t";
                cJSON_AddStringToObject(monitor2, item[j].c_str(), row[j]);
            }
            std::cout << std::endl;
            cJSON_AddItemToObject(monitor, "data", monitor2);
            cJSON_AddStringToObject(monitor, "msg", "login success");
            cJSON_AddNullToObject(monitor, "system");
            cJSON_AddNumberToObject(monitor, "statusCode", 200);
        }
        //}
        // std::cout<<__LINE__<<std::endl;
        // name = cJSON_GetObjectItemCaseSensitive(json, "name");
        // pass = cJSON_GetObjectItemCaseSensitive(json, "pass");
        // std::string tmp1 = "这是我的:" + std::string(name->valuestring);
        // cJSON_AddStringToObject(monitor, "name", tmp1.c_str());
        // std::string tmp2 = "这是我的:" + std::string(pass->valuestring);
        // cJSON_AddStringToObject(monitor, "pass", tmp2.c_str());

        // std::cout << "====line:" << __LINE__ << buf << std::endl;
    }

    struct evbuffer *retbuff = nullptr;
    retbuff = evbuffer_new();
    if (!retbuff)
    {
        std::cerr << "====line:" << __LINE__ << "retbuff in nullptr" << std::endl;
        return;
    }
    //evbuffer_add_printf(retbuff, "api/login/login");
    //evbuffer_add_printf(retbuff, name->valuestring);
    evbuffer_add_printf(retbuff, cJSON_Print(monitor));
    evhttp_send_reply(req, HTTP_OK, "no resource", retbuff);
    evbuffer_free(retbuff);
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
    //MyDB::getInstance()->searchValues("select * from user where Username = \"cpp\";");
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
    // evhttp_set_cb(http_server, "/post", http_handler_post, nullptr);
    // evhttp_set_cb(http_server, "/get", http_handler_get, nullptr);
    // evhttp_set_cb(http_server, "/", http_handler_index, nullptr);
    evhttp_set_cb(http_server, "/api/login/login", login, nullptr);

    //循环监听
    event_dispatch();
    //实际上不会释放，代码不会运行到这一步
    evhttp_free(http_server);

    return 0;
}