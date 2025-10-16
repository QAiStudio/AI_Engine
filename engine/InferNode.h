#pragma once
#include"Node.h"
#include <onnxruntime_cxx_api.h>
#include<filesystem>
class OrtNode:public Node
{
protected:
    std::string Device="cpu";//cuda
    int cudaID=0;
    bool WarmUp=false;
    std::string modelPath;
    int Threads=8;
    double Scale=1.0;
    std::string _rootPath;
public:
    OrtNode(std::string str):Node(str)
    {
        type=NodeType::ORT;
        jr.get("aiParams","device",Device);
        jr.get("aiParams","warmUp",WarmUp);
        jr.get("taskParams","rootPath",_rootPath);
        std::string mp;
        jr.get("aiParams","modelPath",mp);
        modelPath=_rootPath+mp;
        jr.get("aiParams","threads",Threads);
        initModel(modelPath);
        if(WarmUp)
            warmUp();
    }
    ~OrtNode(){}
    bool initModel(std::string);
    bool warmUp();
    virtual std::vector<std::vector<int64_t>> getInputDims(){return input_node_dims;}
	virtual std::vector<std::vector<int64_t>> getOutputDims(){return output_node_dims;}
    virtual void execute() override;
    virtual cv::Mat draw(cv::Mat img) override{return img;}
    int64_t getInputLen(int index)
    {
        int64_t len=1;
        for (auto & dim : input_node_dims[index])
        {
            len*=dim;
        }
    }
    bool nullProc(std::shared_ptr<Tensor> data)
    {
        if(!data)//应对没有输入的时候
		{
			std::shared_ptr<std::vector<Ort::Value>> outTensor=nullptr;
			(*results_ptr)[name]=outTensor;
			return true;
		}
        return false;
    }
protected:
	Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "OnnxRT");
	std::unique_ptr<Ort::Session> ort_session = nullptr;
	Ort::SessionOptions sessionOptions = Ort::SessionOptions();
	std::vector<std::string> input_names;
	std::vector<std::string> output_names;
	std::vector<const char*>input_names_chars;
	std::vector<const char*>output_names_chars;
	std::vector<std::vector<int64_t>> input_node_dims; // >=1 outputs
	std::vector<int64> inputLens;
	std::vector<std::vector<int64_t>> output_node_dims; // >=1 outputs
	Ort::MemoryInfo allocator_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
};

class UltraYOLOOrtNode:public OrtNode
{
public:
    UltraYOLOOrtNode(std::string str):OrtNode(str)
    {
    }
    ~UltraYOLOOrtNode(){}
};