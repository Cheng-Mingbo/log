//
// Created by Cheng MingBo on 2023/2/11.
//
#include "Logger.h"
#include <chrono>
#include <iostream>
#include <format>
#include <thread>
#include <filesystem>
#include <source_location>

std::string Logger::LogLevelToString(Logger::LogLevel logLevel) {
    switch (logLevel) {
        case LogLevel::kTrace:
            return "TRACE";
        case LogLevel::kDebug:
            return "DEBUG";
        case LogLevel::kInfo:
            return "INFO";
        case LogLevel::kWarn:
            return "WARN";
        case LogLevel::kError:
            return "ERROR";
        case LogLevel::kFatal:
            return "FATAL";
    }
}

auto Logger::as_local(std::chrono::system_clock::time_point const tp) {
    return std::chrono::zoned_time{ std::chrono::current_zone(), tp };
}

std::string Logger::to_string(auto tp) {
    return std::format("{:%F %T %Z}", tp);
}

std::string Logger::to_string(std::source_location const source) {
    return std::format("{}:{}:{}",
                       std::filesystem::path(source.file_name()).filename().string(),
                       source.function_name(),
                       source.line());
}

void Logger::Write(Logger::LogLevel logLevel, std::string_view msg, std::string_view thread_id, const std::source_location &location) {
    using namespace std::chrono;
    if (logLevel < log_level_) {
        return;
    }
    if (!file_stream_.is_open()) {
        file_stream_.open(file_name_, std::ios::out | std::ios::app);
        if (!file_stream_.is_open()) {
            std::cerr << "Failed to open file " << file_name_ << " for logging" << std::endl;
            return;
        }
    }
    
    std::ostringstream oss;
    
    oss << std::format("[ {:<6}] [{:<6}] [{:<6}] [{:<23}] {:<}\n", LogLevelToString(logLevel),
                       to_string(as_local(system_clock::now())), thread_id, to_string(location), msg
                      );
    
    file_stream_ << oss.str();
}

void Logger::Trace(std::string_view msg, std::source_location location) {
    auto threadID = GetThreadID();
    auto task = [ msg, threadID, location, this ] { return this->Write(LogLevel::kTrace, msg, threadID, location); };
    {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_.emplace(std::move(task));
    }
    queue_cv_.notify_one();
}

void Logger::Debug(std::string_view msg, std::source_location location) {
    auto threadID = GetThreadID();
    auto task = [ msg, threadID, location, this ] { return this->Write(LogLevel::kDebug, msg, threadID, location); };
    {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_.emplace(std::move(task));
    }
    queue_cv_.notify_one();
}

void Logger::Info(std::string_view msg, std::source_location location) {
    auto threadID = GetThreadID();
    auto task = [ msg, threadID, location, this ] { return this->Write(LogLevel::kInfo, msg, threadID, location); };
    {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_.emplace(std::move(task));
    }
    queue_cv_.notify_one();
}

void Logger::Warn(std::string_view msg, std::source_location location) {
    auto threadID = GetThreadID();
    auto task = [ msg, threadID, location, this ] { return this->Write(LogLevel::kWarn, msg, threadID, location); };
    {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_.emplace(std::move(task));
    }
    queue_cv_.notify_one();
}

void Logger::Error(std::string_view msg, std::source_location location) {
    auto threadID = GetThreadID();
    auto task = [ msg, threadID, location, this ] { return this->Write(LogLevel::kError, msg, threadID, location); };
    {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_.emplace(std::move(task));
    }
    queue_cv_.notify_one();
}

void Logger::Fatal(std::string_view msg, std::source_location location) {
    auto threadID = GetThreadID();
    auto task = [ msg, threadID, location, this ] { return this->Write(LogLevel::kFatal, msg, threadID, location); };
    {
        std::unique_lock<std::mutex> lk(mtx_);
        queue_.emplace(std::move(task));
    }
    queue_cv_.notify_one();
}

void Logger::ThreadFunc() {
    //int count = 0;
    while (running_ || !queue_.empty()) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            queue_cv_.wait(lk, [ this ] {
                return !this->running_ || !this->queue_.empty();
            });
            
            if (!running_ && queue_.empty()) {
                return;
            }
            
            task = std::move(queue_.front());
            queue_.pop();
        }
        task();
        //std::cout << ++count << "\n";
    }
}

void Logger::Stop() {
    {
        std::unique_lock<std::mutex> lk(mtx_);
        running_ = false;
    }
    queue_cv_.notify_all();
    thread_.join();
}

Logger::Logger() : thread_([ this ] { this->ThreadFunc(); }), log_level_(LogLevel::kInfo), running_(true) {
    file_name_ = to_string(as_local(std::chrono::system_clock::now()));
}

std::string Logger::GetThreadID() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

