#pragma once
#include"Node.h"
class PreNode:public Node
{
protected:
    std::string srcKey="src";
    int outW=640;
    int outH=640;
    float Scale=1.0;
    double PixNorm=1.0/255.0;
	cv::Mat _resizeM;
	cv::Mat _borderM;
    std::string _imageFrom;
    int _batchSize=1;
public:
    PreNode(std::string str):Node(str)
    {
        type=NodeType::PRE;
        jr.get("aiParams","outW",outW);
        jr.get("aiParams","outH",outH);
        jr.get("aiParams","imageFrom",_imageFrom);
    }
    ~PreNode(){}
    std::shared_ptr<Tensor> getInput();
    virtual void execute() override;
    virtual cv::Mat draw(cv::Mat) override;
    virtual std::shared_ptr<Tensor> execute(std::shared_ptr<Tensor>)=0;
    virtual cv::Mat preProc(std::shared_ptr<Tensor>);
    cv::Rect RoiWithEdge(const cv::Mat& src,const cv::Rect& roi)
    {
        int imgw=src.cols;
        int imgh=src.rows;
        int x=std::min(std::max(roi.x,0),imgw);
        int y=std::min(std::max(roi.y,0),imgh);
        int w=std::max(std::min(roi.x+roi.width,imgw)-x,0);
        int h=std::max(std::min(roi.y+roi.height,imgh)-y,0);
        return cv::Rect(x,y,w,h);
    }
};

class RGBNode:public PreNode
{
protected:
    std::vector<double> Mean={0.0,0.0,0.0};
    std::vector<double> Std={1.0,1.0,1.0};
    bool isMaD=false;
public:
    RGBNode(std::string str):PreNode(str)
    {
        jr.get("aiParams","isMaD",isMaD);
        jr.get("aiParams","mean",Mean);
        jr.get("aiParams","std",Std);
        cv::setNumThreads(1);
    }
    ~RGBNode(){}
    //virtual std::any getResult() override;
    virtual std::shared_ptr<Tensor> execute(std::shared_ptr<Tensor>) override;
};
class PlateOCRNode:public RGBNode
{
protected:
    std::vector<double> Mean={0.0,0.0,0.0};
    std::vector<double> Std={1.0,1.0,1.0};
    bool isMaD=false;
    std::string _batchFrom;
public:
    PlateOCRNode(std::string str):RGBNode(str)
    {
        jr.get("aiParams","batchROIFrom",_batchFrom);
    }
    ~PlateOCRNode(){}
    virtual cv::Mat preProc(std::shared_ptr<Tensor>)override;
    cv::Mat get_split_merge(const cv::Mat&);
};
