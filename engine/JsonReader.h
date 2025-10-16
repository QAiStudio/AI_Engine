#pragma once
#include <iostream>
#include<json/json.h>
#include <vector>
#include <string>
#include <sstream>
#include"myCommon.h"
class JsonReader
{
private:
    std::string jsonStr="";
    Json::Value Root;
    Json::Reader Reader;
public:
    JsonReader()
    {
    }
    JsonReader(std::string jStr)
    {
        setJson(jStr);
    }
    bool setJson(std::string jStr)
    {
        try
        {
            jsonStr=jStr;
            if (!Reader.parse(jsonStr, Root)) {
            std::cerr << "Failed to parse JSON: " << Reader.getFormattedErrorMessages() << std::endl;
            }
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr<<"Json解析出错:" << e.what() << '\n';
            return false;
        }
    }
    std::string getJson()
    {
        return jsonStr;
    }
    bool isArray(std::string keyStr)
    {
        std::vector<std::string> splitJson;
        const Json::Value& aiParamsArray = Root[keyStr];
        return aiParamsArray.isArray();
    }
    std::vector<std::string> split(const std::string& keyStr)
    {
        std::vector<std::string> splitJson;
        const Json::Value& aiParamsArray = Root[keyStr];
        // 获取 taskParams 的值
        const Json::Value& taskParams = Root["taskParams"];
        if (!aiParamsArray.isArray()) {
            splitJson.emplace_back(jsonStr);
            return splitJson;
        }
    
        // 复制除 aiParams 外的所有字段到基础模板
        Json::Value baseRoot;
        for (const auto& key : Root.getMemberNames()) {
            if (key != keyStr) {
                baseRoot[key] = Root[key];
            }
        }
    
        // 使用 FastWriter 来快速生成输出字符串
        Json::FastWriter fastWriter;
    
        for (unsigned int i = 0; i < aiParamsArray.size(); ++i) {
            Json::Value newRoot = baseRoot;
            newRoot["taskParams"] = taskParams;
            newRoot[keyStr] = aiParamsArray[i];
    
            std::string outputString = fastWriter.write(newRoot);
            splitJson.emplace_back(outputString);
        }
        return splitJson;
    }
    
    bool isExit(std::string subkey)
    {
        if(Root.isMember(subkey)&&!Root[subkey].empty())
            return true;
        return false;
    }
    std::pair<std::string, std::string> splitByDot(const std::string& str) {
        // 查找第一个'$'的位置
        size_t dollarPos = str.find('$');
        std::string processedStr;

        if (dollarPos == std::string::npos) {
            // 没有'$'，使用原字符串
            processedStr = str;
        } else {
            // 保留'$'及其之后的字符串（去除$之前的内容）
            processedStr = str.substr(dollarPos+1);
        }

        // 按 '.' 分割处理后的字符串
        size_t dotPos = processedStr.find('.');
        if (dotPos == std::string::npos) {
            return {processedStr, ""};
        }

        std::string first = processedStr.substr(0, dotPos);
        std::string second = processedStr.substr(dotPos + 1);
        return {first, second};
    }

    bool get(std::string subRootKey, const std::string& key,bool& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isBool())
        {
            value=root[key].asBool();
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key,int& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isInt())
        {
            value=root[key].asInt();
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    float get(std::string subRootKey, const std::string& key,float& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isDouble())
        {
            value=root[key].asFloat();
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    float get(std::string subRootKey, const std::string& key,double& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isDouble())
        {
            value=root[key].asDouble();
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key,std::string& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isString())
        {
            value=root[key].asString();
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key,std::vector<int>& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isArray())
        {
            Json::Value indexes = root[key];
            value.clear();
            for (const auto& index : indexes) {

                value.push_back(index.asInt());
            }
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key,std::vector<double>& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isArray())
        {
            Json::Value indexes = root[key];
            value.clear();
            for (const auto& index : indexes) {

                value.push_back(index.asDouble());
            }
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key,std::vector<std::string>& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isArray())
        {
            value.clear();
            const Json::Value array = root[key];
            for (Json::ArrayIndex i = 0; i < array.size(); ++i) {
                value.push_back(array[i].asString());
            }
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key,const std::string& arrayKey,std::vector<std::string>& value)
    {
        Json::Value root;
        if(subRootKey=="")
            root=Root;
        else 
            root=Root[subRootKey];
        if(root.isMember(key)&&root[key].isArray())
        {
            value.clear();
            const Json::Value array = root[key];
            for (Json::ArrayIndex i = 0; i < array.size(); ++i) {
                value.push_back(array[i][arrayKey].asString());
            }
            return true;
        }
        else if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        else
            return false;
    }
    bool get(std::string subRootKey, const std::string& key, std::vector<cvResult>& value)
    {
        Json::Value root;
        if (subRootKey.empty())
            root = Root;
        else 
            root = Root[subRootKey];
        value.clear();//避免传入为空时，先清空
        if(root.isMember(key)&&root[key].isString()&&root[key].asString().find("$")!= std::string::npos)
        {
            auto pair=splitByDot(root[key].asString());
            get(pair.first,pair.second,value);
            return true;
        }
        if (!root.isMember(key) || !root[key].isArray())
            return false;
    
        
        const Json::Value mainArray = root[key];
    
        // 检查数组层级：判断是单层点集还是双层点集
        bool isTwoLevel = false;
        if (mainArray.size() > 0)
        {
            const Json::Value firstElem = mainArray[0];
            // 如果第一层元素是数组，且该数组的元素还是数组，则为双层结构
            if (firstElem.isArray() && firstElem.size() > 0 && firstElem[0].isArray())
            {
                isTwoLevel = true;
            }
        }
    
        if (isTwoLevel)
        {
            // 处理双层点集：[[[x1,y1],...], [[x2,y2],...]]
            for (Json::ArrayIndex i = 0; i < mainArray.size(); ++i)
            {
                const Json::Value polygonArray = mainArray[i];
                if (!polygonArray.isArray()) continue;
    
                std::vector<cv::Point> polygon;
                for (Json::ArrayIndex j = 0; j < polygonArray.size(); ++j)
                {
                    const Json::Value pointArray = polygonArray[j];
                    if (pointArray.isArray() && pointArray.size() == 2)
                    {
                        polygon.emplace_back(
                            cv::Point(pointArray[0].asInt(), pointArray[1].asInt())
                        );
                    }
                }
                if (!polygon.empty())
                    value.emplace_back(cvResult(0, 1.0F, polygon, "roi", std::array<int, 3>{0, 255, 0}));
            }
        }
        else
        {
            // 处理单层点集：[[x1,y1], [x2,y2], ...]（视为单个多边形）
            std::vector<cv::Point> polygon;
            for (Json::ArrayIndex i = 0; i < mainArray.size(); ++i)
            {
                const Json::Value pointArray = mainArray[i];
                if (pointArray.isArray() && pointArray.size() == 2)
                {
                    polygon.emplace_back(
                        cv::Point(pointArray[0].asInt(), pointArray[1].asInt())
                    );
                }
            }
            if (!polygon.empty())
                value.emplace_back(cvResult(0, 1.0F, polygon, "roi", std::array<int, 3>{0, 255, 0}));
        }
    }
    // 解析字符串为整数数组
    std::vector<int> parseStringToArray(const std::string& str) {
        std::vector<int> result;
        std::stringstream ss(str.substr(1, str.length() - 2)); // 去掉方括号
        std::string token;
    
        while (std::getline(ss, token, ',')) {
            try {
                result.push_back(std::stoi(token));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid number: " << token << std::endl;
            }
        }
    
        return result;
    }
    
    // 将字符串数组赋值到 JSON 对象
    Json::Value assignStringArrayToJson(const std::string& str) {
        Json::Value jsonArray(Json::arrayValue);
        std::vector<int> array = parseStringToArray(str);
    
        for (int num : array) {
            jsonArray.append(num);
        }
    
        return jsonArray;
    }
    bool set(std::string subRootKey, const std::string& key,std::string value)
    {
        Json::Value* root;
        if(subRootKey=="")
            root=&Root;
        else 
            root=&Root[subRootKey];
        if(root->isMember(key))
        {   if((*root)[key].isString())
            {
                (*root)[key]=value;
            }
            else if((*root)[key].isArray())
            {
                (*root)[key] = assignStringArrayToJson(value);
            }
            else if((*root)[key].isInt())
            {
                (*root)[key]=std::atoi(value.c_str());
            }
            return true;
        }
        else
            return false;
    }
    std::string dump()
    {
        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, Root);
    }
    ~JsonReader(){}
};


