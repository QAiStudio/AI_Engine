#pragma once
#include<iostream>
#include<any>
#include <atomic>
#include <vector>
#include <memory>
#include <unordered_set>
#include<map>
#include<tbb/tbb.h>
#include "qLog.h"
#include"JsonReader.h"
#include<opencv2/opencv.hpp>

class Node
{
private:
    std::atomic<int> counter=0; // 节点ID
protected:
    std::map<std::string,std::shared_ptr<Node>>* nodes_ptr=nullptr;
    tbb::concurrent_unordered_map<std::string,std::any>* results_ptr=nullptr;
    std::string name;
    std::string className;
    std::string buildStr;
    std::shared_ptr<tbb::task_group> tg=nullptr; // TBB任务组，用于并行执行节点
    std::shared_ptr<qLog> myLog=nullptr; // 日志实例
    JsonReader jr;
    NodeType type=NodeType::PRE;
    double boxScale=1.0;
public:
    Node(std::string);
    ~Node();
    virtual void run(); // 运行节点的虚函数，接受任意类型的数据
    virtual void execute()=0; // 执行节点的具体操作
    virtual void next();
    virtual cv::Mat draw(cv::Mat)=0;
    //virtual std::any getResult()=0; // 获取节点的结果
    double getBoxScale()
    {
        return boxScale;
    }
    void setNodes(std::map<std::string,std::shared_ptr<Node>>* nodesPtr)
    {
        nodes_ptr=nodesPtr;
    }
    void setResults(tbb::concurrent_unordered_map<std::string,std::any>* resultsPtr)
    {
        results_ptr=resultsPtr;
    }
    void setTg(std::shared_ptr<tbb::task_group> taskGroup)
    {
        tg=taskGroup;
    }
    void setLog(std::shared_ptr<qLog> log)
    {   
        myLog=log;
    }
    std::string getName()
    {
        return name;
    }
    std::string getClassName()
    {
        return className;
    }
    std::string getBuildStr()
    {
        return buildStr;
    }
public:
    std::unordered_set<std::string> Froms;

    std::unordered_set<std::string> Tos;
};
