#include "sql.h"

MyDB::MyDB(/* args */)
{
    mysql = mysql_init(nullptr);
}

MyDB::~MyDB()
{
    if (mysql != nullptr)
    {
        mysql_close(mysql);
    }
    if (result != nullptr)
    {
        mysql_free_result(result);
    }
}

MyDB *MyDB::getInstance()
{
    static MyDB instance;
    return &instance;
}
//连接数据库
bool MyDB::connect(std::string host, std::string user, std::string passwd, std::string database)
{
    mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), database.c_str(), 8086, nullptr, 0);
    if (mysql == nullptr)
    {
        std::cout << "connect database failed!" << std::endl;
        return false;
    }
    else
    {
        return true;
    }
}

//插入数据
bool MyDB::insertValues(std::string sql)
{
    int res = mysql_query(mysql, sql.c_str());
    if (res == 0)
    {
        std::cout << "insertValues successed!" << std::endl;
        return true;
    }
    else
    {
        std::cout << "insertValues failed!" << std::endl;
        return false;
    }
    return false;
}
//删除数据
bool MyDB::delValues(std::string sql)
{
    int res = mysql_query(mysql, sql.c_str());
    if (res == 0)
    {
        std::cout << "delValues successed!" << std::endl;
        return true;
    }
    else
    {
        std::cout << "delValues failed!" << std::endl;
        return false;
    }
    return false;
}
//查询数据
bool MyDB::searchValues(std::string sql)
{
    int res = mysql_query(mysql, sql.c_str());
    if (res == 0)
    {
        std::cout << "serchValues successed!" << std::endl;
        //return true;
    }
    else
    {
        std::cout << "serchValues failed!" << std::endl;
        //return false;
    }
    result = mysql_store_result(mysql);
    if (result) // 返回了结果集
    {
        int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
        int num_rows = mysql_num_rows(result);     //获取结果集中总共的行数
        for (int i = 0; i < num_rows; i++)         //输出每一行
        {
            //获取下一行数据
            row = mysql_fetch_row(result);
            if (row < 0)
                break;

            for (int j = 0; j < num_fields; j++) //输出每一字段
            {
                std::cout << row[j] << "\t\t";
            }
            std::cout << std::endl;
        }
        return true;
    }
    return false;
}
//修改数据
bool MyDB::updateValues(std::string sql)
{
    int res = mysql_query(mysql, sql.c_str());
    if (res == 0)
    {
        std::cout << "updateValues successed!" << std::endl;
        return true;
    }
    else
    {
        std::cout << "updateValues failed!" << std::endl;
        return false;
    }
    return false;
}

//查询数据
MYSQL_RES *MyDB::searchValuesToRes(std::string sql)
{
    int res = mysql_query(mysql, sql.c_str());
    if (res == 0)
    {
        std::cout << "serchValues successed!" << std::endl;
        //return true;
    }
    else
    {
        std::cout << "serchValues failed!" << std::endl;
        return nullptr;
    }
    result = mysql_store_result(mysql);
    if (result->row_count > 0)
        return result;
    else
        return nullptr;
}
