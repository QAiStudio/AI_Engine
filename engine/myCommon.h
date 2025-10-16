#pragma once
#include <iostream>
#include<json/json.h>
#include <vector>
#include <string>
#include<opencv2/opencv.hpp>
#include <tbb/task_arena.h>
#include <tbb/task_group.h>
#include <vector>
#include <thread>

#include <fstream>
#include <sstream>
#include <algorithm>

enum NodeType
{
    PRE,
    ORT,
    TRT,
    POST,
    TRACK,
    EVENT
};
enum TensorType
{
    F32,
    UC8,
};
struct Tensor {
    TensorType type=TensorType::F32;
    cv::Mat Mat;
    void* Data=nullptr;  // 存储数据的向量
    std::vector<int> Shape; // 存储张量的形状
    size_t FrameID=0;
    int64_t TimePoint = 0;
    int BatchSize=1;
    int FPS=0;
    // 构造函数，根据形状初始化数据
    Tensor()
    {

    }
    Tensor(const std::vector<int>& shape, float* imgdata = nullptr){
        Shape=shape;
        auto len=getLength();
        if (imgdata)
        {
            Data = new float[len];
            memcpy(Data, imgdata, len*sizeof(float));
        }
        else
            Data = new float[len];
    }
    Tensor(const std::vector<int>& shape, unsigned char* imgdata = nullptr){
        type=TensorType::UC8;
        Shape=shape;
        auto len=getLength();
        if (imgdata)
        {
            Data = new unsigned char[len];
            memcpy(Data, imgdata, len*sizeof(unsigned char));
        }
        else
            Data = new unsigned char[len];
    }
    Tensor(const std::vector<int>& shape,size_t frameID,int64_t tp,int fps, float* imgdata = nullptr) {
        Shape = shape;
        FrameID = frameID;
        TimePoint = tp;
        FPS=fps;
        auto len=getLength();
        if (imgdata)
        {
            Data = new float[len];
            memcpy(Data, imgdata, len * sizeof(float));
        }
        else
            Data = new float[len];
    }
    Tensor(const std::vector<int>& shape,size_t frameID,int64_t tp,int fps, unsigned char* imgdata = nullptr) {
        type=TensorType::UC8;
        Shape = shape;
        FrameID = frameID;
        TimePoint = tp;
        FPS=fps;
        auto len=getLength();
        if (imgdata)
        {
            Data = new unsigned char[len];
            memcpy(Data, imgdata, len * sizeof(unsigned char));
        }
        else
            Data = new unsigned char[len];
    }
    ~Tensor()
    {
        if (Data)
        {
            if(Data==Mat.data)
            {
                Data=nullptr;
                return;
            }
            if(type==TensorType::F32)
                delete[] static_cast<float*> (Data);
            else
            {
                delete[] static_cast<unsigned char*> (Data);
            }
            Data = nullptr;
        }
    }
    std::shared_ptr<Tensor> clone()
    {
        if (type==TensorType::F32)
        {
            auto tensor=std::make_shared<Tensor>(Shape,FrameID,TimePoint,FPS,static_cast<float*>(Data));
            tensor->type=type;
            return tensor;
        }
        else
        {
            auto tensor=std::make_shared<Tensor>(Shape,FrameID,TimePoint,FPS,static_cast<unsigned char*>(Data));
            tensor->type=type;
            return tensor;
        }
    }
    std::shared_ptr<Tensor> clone(cv::Mat mat,int bs=1)
    {
        auto tensor=std::make_shared<Tensor>();
        if(mat.depth() == CV_32F)
        {
            tensor->Data=mat.ptr<float>();
            tensor->type=TensorType::F32;
        }
        else
        {
            tensor->Data=mat.data;
            tensor->type=TensorType::UC8;
        }
        tensor->Mat=mat;
        tensor->BatchSize=bs;
        if (bs==1)
            tensor->Shape={mat.rows,mat.cols,mat.channels()};
        else
            tensor->Shape={bs,mat.rows/bs,mat.cols,mat.channels()};
        tensor->FrameID=FrameID;
        tensor->FPS=FPS;
        tensor->TimePoint=TimePoint;
        return tensor;
    }
    int getW()
    {
        return this->BatchSize==1?this->Shape[1]:this->Shape[2];
    }
    int getH()
    {
        return this->BatchSize==1?this->Shape[0]:this->Shape[1];
    }
    int getCh()
    {
        return this->BatchSize==1?this->Shape[2]:this->Shape[3];
    }
    int getBS()
    {
        return this->BatchSize;
    }
    size_t getLength() const {
        size_t len = 1;
        for (const int& d : Shape)
            len *= d;
        return len ;
    }
};
struct cvResult
{
	int X{};
	int Y{};
	int W{};
	int H{};
	float Score{};
	int ClassID{};
    int TrackID=0;
    std::array<int, 3> Color;
    std::string Name;
    std::vector<cv::Point> polygon;
    cvResult()
	{
	}
	cvResult(int classId, float conf, int ltx, int lty, int w, int h,std::string name,std::array<int, 3> color)
	{
		ClassID = classId; Score = conf; X = ltx; Y = lty; W = w; H = h;Name=name;Color=color; 
	}
    cvResult(int classId, float conf,const std::vector<cv::Point>& poly,std::string name,std::array<int, 3> color)
	{
		ClassID = classId; Score = conf;polygon=poly;Name=name;Color=color; 
	}
    cvResult(int classId, float conf, int ltx, int lty, int w, int h,const std::vector<cv::Point>& poly,std::string name,std::array<int, 3> color)
	{
		ClassID = classId; Score = conf; X = ltx; Y = lty; W = w; H = h;Name=name;Color=color; polygon=poly;
	}
    cvResult(int classId, float conf, int ltx, int lty, int w, int h,std::string name,std::array<int, 3> color,int trackID)
	{
		ClassID = classId; Score = conf; X = ltx; Y = lty; W = w; H = h;Name=name;Color=color; TrackID=trackID;
	}
    size_t getArea() const
    {
        return W*H;
    }
    cv::Rect bbox()
    {
        if(polygon.size()>3)
            return cv::boundingRect(polygon);
        else
            return cv::Rect(X,Y,W,H);
    }
    bool isContainBox(const cvResult& other) const
    {
        if(polygon.size()>3)
            return cv::pointPolygonTest(polygon,cv::Point2f(other.X+other.W/2.0,other.Y+other.H/2.0),false)>=0;
        else
            return other.X+other.W/2.0>=X && other.Y+other.H/2.0>=Y && (other.X+other.W)<=(X+W) && (other.Y+other.H)<=(Y+H);
    }
    bool getCenter(float& px ,float& py)
    {
        px=X+W/2;
        py=Y+H/2;
        return true;
    }

    void Offset(size_t x,size_t y)
    {
        X+=x;
        Y+=y;
    }
    cvResult OffsetNew(size_t x,size_t y) const {
        return cvResult(
            ClassID,
            Score,
            X + x,
            Y + y,
            W,
            H,
            Name,
            Color
        );
    }
};

class SystemMonitor {
private:
    struct CoreLoad {
        int core_id;
        double load;
        bool is_physical; // 是否为物理核心
        
        CoreLoad(int id, double l, bool phys) 
            : core_id(id), load(l), is_physical(phys) {}
    };
    
    std::vector<CoreLoad> last_loads_;

public:
    // 获取所有核心的负载（基于/proc/stat）
    std::vector<CoreLoad> get_core_loads() {
        std::vector<CoreLoad> loads;
        std::ifstream file("/proc/stat");
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.rfind("cpu", 0) == 0 && std::isdigit(line[3])) {
                std::istringstream iss(line);
                std::string cpu;
                long user, nice, system, idle, iowait, irq, softirq;
                
                iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
                
                int core_id = std::stoi(cpu.substr(3));
                long total = user + nice + system + idle + iowait + irq + softirq;
                long work = user + nice + system + irq + softirq;
                double load = (double)work / total;
                
                // 判断是否为物理核心（简化：偶数编号为核心，奇数为HT）
                bool is_physical = (core_id % 2 == 0);
                
                loads.emplace_back(core_id, load, is_physical);
            }
        }
        
        return loads;
    }
    
    // 获取最空闲的N个物理核心
    std::vector<int> get_least_busy_cores(int n) {
        auto loads = get_core_loads();
        
        // 只保留物理核心并按负载排序
        std::vector<CoreLoad> physical_only;
        for (const auto& load : loads) {
            if (load.is_physical) {
                physical_only.push_back(load);
            }
        }
        
        std::sort(physical_only.begin(), physical_only.end(),
                  [](const CoreLoad& a, const CoreLoad& b) {
                      return a.load < b.load; // 负载低的在前
                  });
        
        // 选择前N个最空闲的
        std::vector<int> result;
        int count = std::min(n, (int)physical_only.size());
        for (int i = 0; i < count; ++i) {
            result.push_back(physical_only[i].core_id);
        }
        
        return result;
    }
};

#include <tbb/task_arena.h>
#include <tbb/task_group.h>
#include <vector>
#include <thread>
#include <functional>
#include <iostream>
#include <sched.h>
#include <sys/syscall.h>

class SmartThreadPool {
private:
    tbb::task_arena arena_;
    tbb::task_group tg_;
    std::vector<int> assigned_cores_;
    std::vector<std::thread> worker_threads_;
    bool initialized_ = false;

public:
    SmartThreadPool(int requested_cores) {
        // 1. 获取最空闲的物理核心
        assigned_cores_ = get_least_busy_physical_cores(requested_cores);
        
        if (assigned_cores_.empty()) {
            throw std::runtime_error("No available cores found");
        }
        
        std::cout << "[SmartPool] Assigned " << assigned_cores_.size() 
                  << " cores: ";
        for (int c : assigned_cores_) {
            std::cout << c << " ";
        }
        std::cout << std::endl;
        
        // 2. 初始化arena（不带初始化函数）
        int max_threads = static_cast<int>(assigned_cores_.size());
        arena_.initialize(max_threads, 1U); // 只用前两个参数
        
        // 3. 手动创建并绑定工作线程
        start_worker_threads();
        
        initialized_ = true;
    }
    
    ~SmartThreadPool() {
        stop_worker_threads();
    }
    
    template<typename F>
    void run(F&& func) {
        if (!initialized_) {
            throw std::runtime_error("Thread pool not initialized");
        }
        
        arena_.enqueue([this, func = std::forward<F>(func)] {
            tg_.run(func);
        });
    }
    
    void wait() {
        if (!initialized_) return;
        tg_.wait();
    }

private:
    void start_worker_threads() {
        for (size_t i = 0; i < assigned_cores_.size(); ++i) {
            worker_threads_.emplace_back([this, i]() {
                // 绑定到指定核心
                bind_to_core(assigned_cores_[i]);
                
                // 让这个线程参与TBB的任务调度
                arena_.execute([]{
                    // 这个lambda会让线程加入TBB的工作队列
                    while (true) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                });
            });
        }
    }
    
    void stop_worker_threads() {
        // 停止所有工作线程
        for (auto& thread : worker_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        worker_threads_.clear();
    }
    
    static void bind_to_core(int core_id) {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(core_id, &mask);
        int result = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
        if (result != 0) {
            std::cerr << "Failed to set CPU affinity for core " << core_id << std::endl;
        }
        
        std::cout << "[Bind] Thread TID=" << syscall(SYS_gettid)
                  << " -> Core " << core_id << std::endl;
    }
    
    std::vector<int> get_least_busy_physical_cores(int n) {
        // 实际应用中应实现真正的负载检测
        // 这里返回前n个偶数编号的核心（避免超线程）
        std::vector<int> cores;
        // 根据您的CPU信息，有72物理核心
        for (int i = 0; i < 72 && cores.size() < n; i += 2) {
            cores.push_back(i);
        }
        return cores;
    }
};