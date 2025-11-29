#include "../include/Alogger.h"

ALogger::ALogger()
{
    std::pair<std::string, mylog::LogLevel> configurationLog_ = mylog::Config::GetInstance().ReadConfig("LogSystem.conf");
    std::shared_ptr<mylog::LoggerBuilder> Glb = std::make_shared<mylog::LoggerBuilder>();
    Glb->BuildLoggerName("asynclogger");
    Glb->BuildLoggerFlush<mylog::RollFileFlush>("app.log", 1024);
    mylog::LoggerManager::GetInstance().AddLogger(Glb->Build(mylog::LogLevel::DEBUG));

    logPtr_ = mylog::LoggerManager::GetInstance().GetLogger("asynclogger");
    logPtr_->Info(configurationLog_.first);
}

void ALogger::DEBUG(const std::string& unformatted_message)
{
    logPtr_->Debug(unformatted_message);
}

void ALogger::INFO(const std::string& unformatted_message)
{
    logPtr_->Info(unformatted_message);
}

void ALogger::WARN(const std::string& unformatted_message)
{
    logPtr_->Warn(unformatted_message);
}

void ALogger::ERROR(const std::string& unformatted_message)
{
    logPtr_->Error(unformatted_message);
}

void ALogger::FATAL(const std::string& unformatted_message)
{
    logPtr_->Fatal(unformatted_message);
}



ALogger* logger_ = new ALogger();