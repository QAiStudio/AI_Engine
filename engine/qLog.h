#pragma once
#include<iostream>
#include<sstream>
#include "spdlog/spdlog.h"
#include "spdlog/async.h" 
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <filesystem>
class qLog
{
public:
    qLog(std::string taskID,std::string sn,std::string logPath="/wingo/AILogs/")
    {
        try {
            #pragma region sync
            // std::filesystem::path fullPath = logPath + getYMD() + "/" + taskID;
            // if (!std::filesystem::exists(fullPath)) {
            //     std::filesystem::create_directories(fullPath);  // 递归创建目录
            // }
            // Name=sn;
            // // 创建一个轮换文件 sink，设置文件大小为5MB，最多保留3个文件
            // auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath +getYMD()+"/"+ taskID+"/runLog.log", 5 * 1024 * 1024, 5);
            // mylogger = std::make_shared<spdlog::logger>(Name, rotating_sink);
            // spdlog::register_logger(mylogger);
            // spdlog::set_default_logger(mylogger);
            #pragma endregion
            #pragma region async
            std::filesystem::path fullPath = logPath + getYMD() + "/" + taskID;
            if (!std::filesystem::exists(fullPath)) {
                std::filesystem::create_directories(fullPath);  // 递归创建目录
            }
            Name = sn;
            // 创建一个轮换文件 sink
            auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath + getYMD() + "/" + taskID + "/runLog.log", 
                5 * 1024 * 1024,  // 单个文件最大5MB
                5                  // 最多保留5个文件
            );
            spdlog::init_thread_pool(8192, 1);
            // 创建异步 logger，队列大小默认为 8192，也可以指定
            mylogger = std::make_shared<spdlog::async_logger>(
                Name, 
                rotating_sink,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
            // 注册为全局 logger（可选）
            spdlog::register_logger(mylogger);
            // 设置为默认 logger（可选）
            spdlog::set_default_logger(mylogger);
            #pragma endregion
        } catch (const std::exception& e) {
            std::cerr <<Name<< " :log初始化失败 " << e.what() << std::endl; // 打印错误信息
        }
    }
public:
    std::string getYMD() {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
        // 转换为time_t类型，以便可以使用C语言的时间函数
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    
        // 创建一个tm结构体的实例
        std::tm now_tm;
        // 使用localtime函数来填充tm结构体
        localtime_r(&now_c, &now_tm); // 注意：localtime_r在某些编译器或平台上可能不可用
    
        // 提取年、月、日和小时
        int year = now_tm.tm_year + 1900; // tm_year是从1900年起的年数
        int month = now_tm.tm_mon + 1;    // tm_mon是从0开始的月份
        int day = now_tm.tm_mday;
        int hour = now_tm.tm_hour;
        // std::stringstream ss;
        // ss << year<<"_" << month<<"_" << day <<"_"<< hour;
        return std::to_string(year)+"_"+std::to_string(month)+"_"+std::to_string(day)+"_"+std::to_string(hour);
    }
    bool Info(const std::string& info,bool isWrite=false)
    {
        try
        {
            _ss<<info;
            if(isWrite)
            {
                mylogger->info(_ss.str());
                _ss.str("");  // 清空内容但保留缓冲区
                _ss.clear();  // 重置状态标志
            }
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr <<Name<<" : "<< e.what() << '\n';
            return false;
        }
    }
    bool Error(const std::string& error)
    {
        try
        {
            mylogger->error(error);
            mylogger->flush();
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr <<Name<<" : "<< e.what() << '\n';
            return false;
        }
    }
    bool Warn(const std::string& error)
    {
        try
        {
            mylogger->warn(error);
            mylogger->flush();
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr <<Name<<" : "<< e.what() << '\n';
            return false;
        }
    }
    bool Flush()
    {
        try
        {
            mylogger->flush();
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr <<Name<<" : "<< e.what() << '\n';
            return false;
        }
    }
protected:
    std::shared_ptr<spdlog::logger> mylogger=nullptr;
    std::stringstream _ss;
private:
    std::string Name;
};