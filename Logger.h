//
// Created by Cheng MingBo on 2023/2/11.
//

#ifndef LOGGING_LOGGER_H
#define LOGGING_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <source_location>
#include <queue>
#include <atomic>
#include <functional>

class Logger {
  public:
    enum class LogLevel {
        kTrace, kDebug, kInfo, kWarn, kError, kFatal
    };
    
    Logger(const Logger &) = delete;
    
    Logger &operator=(const Logger &) = delete;
    
    ~Logger() {
        Stop();
        if (file_stream_.is_open()) {
            file_stream_.close();
        }
    }
    
    static Logger &GetInstance() {
        static Logger logger;
        return logger;
    }
    
    void Trace(std::string_view msg, std::source_location location = std::source_location::current());
    
    void Debug(std::string_view msg, std::source_location location = std::source_location::current());
    
    void Info(std::string_view msg, std::source_location location = std::source_location::current());
    
    void Warn(std::string_view msg, std::source_location location = std::source_location::current());
    
    void Error(std::string_view msg, std::source_location location = std::source_location::current());
    
    void Fatal(std::string_view msg, std::source_location location = std::source_location::current());
    
    void set_log_level(LogLevel logLevel) { log_level_ = logLevel; }
    
  private:
    Logger();
    
    void Stop();
    
    static std::string GetThreadID();
    
    static std::string LogLevelToString(LogLevel logLevel);
    
    void Write(LogLevel logLevel, std::string_view msg, std::string_view thread_id, const std::source_location &location);
    
    void ThreadFunc();
    
    static auto as_local(std::chrono::system_clock::time_point tp);
    
    static std::string to_string(auto tp);
    
    static std::string to_string(std::source_location source);
    
    LogLevel log_level_;
    std::atomic<bool> running_;
    std::string file_name_;
    std::ofstream file_stream_;
    std::mutex mtx_;
    std::condition_variable queue_cv_;
    std::queue<std::function<void()>> queue_;
    std::thread thread_;
};


#endif //LOGGING_LOGGER_H
