#include <iostream>
#include "../include/log_system/include/MyLog.hpp"
// #include "../include/Channel.h"

#if 1
using namespace mylog;

void threadHandler1(const char* message, const char* logger_name)
{
    mylog::GetLogger(logger_name)->Debug(message);
}

void threadHandler2(const char* message, const char* logger_name)
{
    mylog::GetLogger(logger_name)->Info(message);
}

void threadHandler3(const char* message, const char* logger_name)
{
    mylog::GetLogger(logger_name)->Warn(message);
}

void threadHandler4(const char* message, const char* logger_name)
{
    mylog::GetLogger(logger_name)->Error(message);
}
void threadHandler5(const char* message, const char* logger_name)
{
    mylog::GetLogger(logger_name)->Fatal(message);
}

int main()
{
    mylog::Config::GetInstance().ReadConfig("LogSystem.conf");
    mylog::LoggerManager::GetInstance().AddDefaultLogger(nullptr);

    // 使用日志器建造者一个名字叫asynclogger的日志器
    // std::shared_ptr<mylog::LoggerBuilder> Glb = std::make_shared<mylog::LoggerBuilder>();
    // Glb->BuildLoggerName("asynclogger");
    // Glb->BuildLoggerFlush<mylog::FileFlush>("./log/app.log", 100 * 1024);
    // Glb->BuildLoggerThreadPool(nullptr);

    // 将日志器添加到日志管理者中，管理者是全局单例类 
    // mylog::LoggerManager::GetInstance().AddLogger(Glb->Build(mylog::LogLevel::DEBUG));   // 小于Warn的Debug和Info不会被写到日志当中
     
    mylog::LoggerManager::GetInstance().GetLogger("default")->Fatal("--------------------------------");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));


    std::vector<std::thread> threads_;
    // threads_.push_back(std::thread(threadHandler1, "梦苻坚将天官使者，鬼兵数百突入营中", "asynclogger"));
    // threads_.push_back(std::thread(threadHandler5, "苌俱，走入宫", "asynclogger"));
    threads_.push_back(std::thread(threadHandler5, "宫人迎苌刺鬼，误中苌阴", "default"));
    // threads_.push_back(std::thread(threadHandler2, "鬼相谓曰：正中死处", "asynclogger"));
    threads_.push_back(std::thread(threadHandler4, "拔矛，出血石余", "default"));
    threads_.push_back(std::thread(threadHandler3, "寐而惊悸，遂患阴肿", "default"));
    // threads_.push_back(std::thread(threadHandler4, "医刺之，出血如梦", "asynclogger"));
    threads_.push_back(std::thread(threadHandler4, "苌遂狂言，或曰", "default"));
    threads_.push_back(std::thread(threadHandler3, "臣苌，杀陛下者兄襄，非臣之罪", "default"));
    // threads_.push_back(std::thread(threadHandler4, "愿陛下不惘臣", "asynclogger"));
    // threads_.push_back(std::thread(threadHandler1, "梦苻坚将天官使者，鬼兵数百突入营中", "asynclogger"));
    // threads_.push_back(std::thread(threadHandler5, "苌俱，走入宫", "asynclogger"));
    threads_.push_back(std::thread(threadHandler5, "宫人迎苌刺鬼，误中苌阴", "default"));
    // threads_.push_back(std::thread(threadHandler5, "鬼相谓曰：正中死处", "asynclogger"));
    threads_.push_back(std::thread(threadHandler5, "拔矛，出血石余", "default"));
    threads_.push_back(std::thread(threadHandler5, "寐而惊悸，遂患阴肿", "default"));
    // threads_.push_back(std::thread(threadHandler5, "医刺之，出血如梦", "asynclogger"));
    threads_.push_back(std::thread(threadHandler4, "苌遂狂言，或曰", "default"));
    threads_.push_back(std::thread(threadHandler3, "臣苌，杀陛下者兄襄，非臣之罪", "default"));
    // threads_.push_back(std::thread(threadHandler4, "愿陛下不惘臣", "asynclogger"));

    for(std::thread& t: threads_)
    {
        t.join();
    }

    std::cerr << "over..." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));  // 冷机
    return 0;
}
#endif