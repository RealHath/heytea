#pragma once
#include <mysql/mysql.h>
#include <iostream>
#include <cjson/cJSON.h>
#include <vector>

class MyDB
{
private:
    /* data */
    MYSQL *mysql;       //用于定义一个mysql对象，便于后续操作确定要操作的数据库是哪一个。
    MYSQL_ROW row;      //用于定义一个行对象，其内容含有一行的数据。
    MYSQL_RES *result;  //用于定义一个存储数据库检索信息结果的对象。
    MYSQL_FIELD *field; //用于定义一个存储字段信息的对象。
private:
    MyDB(/* args */);
    ~MyDB();

public:
    //连接数据库
    bool connect(std::string host, std::string user, std::string passwd, std::string database);
    //插入数据
    bool insertValues(std::string sql);
    //删除数据
    bool delValues(std::string sql);
    //查询数据
    bool searchValues(std::string sql);
    //查询数据
    MYSQL_RES *searchValuesToRes(std::string sql);
    //修改数据
    bool updateValues(std::string sql);

public:
    static MyDB *getInstance();
};
