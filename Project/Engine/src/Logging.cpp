#include "pch.h"
#include "Logging.hpp"

#if __has_include("spdlog/spdlog.h")
    #include "spdlog/spdlog.h" 
    #include "spdlog/sinks/stdout_color_sinks.h"
    #include "spdlog/sinks/basic_file_sink.h"
    #include "spdlog/sinks/base_sink.h"
    #include "spdlog/pattern_formatter.h"
    #define SPDLOG_AVAILABLE 1
#else
    #define SPDLOG_AVAILABLE 0
    #include <iostream>
    #include <fstream>
#endif

#include <filesystem>

namespace EngineLogging {
        
#if SPDLOG_AVAILABLE
    // Custom GUI sink that pushes to thread-safe queue
    class GuiSink : public spdlog::sinks::base_sink<std::mutex> {
    public:
        explicit GuiSink(GuiLogQueue& queue) : m_GuiQueue(queue) {}

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            // Convert spdlog level to our LogLevel enum
            LogLevel level;
            switch (msg.level) {
                case spdlog::level::trace:    level = LogLevel::Trace; break;
                case spdlog::level::debug:    level = LogLevel::Debug; break;
                case spdlog::level::info:     level = LogLevel::Info; break;
                case spdlog::level::warn:     level = LogLevel::Warn; break;
                case spdlog::level::err:      level = LogLevel::Error; break;
                case spdlog::level::critical: level = LogLevel::Critical; break;
                default:                      level = LogLevel::Info; break;
            }
            
            // Format the message
            std::string message = fmt::to_string(msg.payload);
            
            // Push to GUI queue
            m_GuiQueue.Push(LogMessage(message, level));
        }

        void flush_() override {
            // Nothing to flush for GUI sink
        }

    private:
        GuiLogQueue& m_GuiQueue;
    };

    // Static instances
    static std::shared_ptr<spdlog::logger> s_Logger;
#endif
    static GuiLogQueue s_GuiLogQueue;
    static bool s_Initialized = false;

    // GuiLogQueue implementation
    void GuiLogQueue::Push(const LogMessage& message) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        // Remove old messages if queue is full
        while (m_Queue.size() >= MAX_QUEUE_SIZE) {
            m_Queue.pop();
        }
        
        m_Queue.push(message);
    }

    bool GuiLogQueue::TryPop(LogMessage& message) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Queue.empty()) {
            return false;
        }
        
        message = m_Queue.front();
        m_Queue.pop();
        return true;
    }

    void GuiLogQueue::Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::queue<LogMessage> empty;
        m_Queue.swap(empty);
    }

    size_t GuiLogQueue::Size() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Queue.size();
    }

    // Logging system functions
    bool Initialize() {
        if (s_Initialized) {
            return true;
        }

        try {
            // Create logs directory if it doesn't exist
            std::filesystem::create_directories("logs");

#if SPDLOG_AVAILABLE
            // Create sinks
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/engine.log", true);
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            auto gui_sink = std::make_shared<GuiSink>(s_GuiLogQueue);
            gui_sink->set_level(spdlog::level::trace);
            gui_sink->set_pattern("%v"); // Simple pattern for GUI, no timestamp needed

            // Create logger with multiple sinks
            std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink, gui_sink };
            s_Logger = std::make_shared<spdlog::logger>("engine", sinks.begin(), sinks.end());
            s_Logger->set_level(spdlog::level::trace);
            s_Logger->flush_on(spdlog::level::warn);

            // Register as default logger
            spdlog::set_default_logger(s_Logger);
#endif

            s_Initialized = true;
            
            // Log initialization message
            LogInfo("Engine logging system initialized");
            
            return true;
        }
        catch (const std::exception& ex) {
            std::cerr << "Failed to initialize logging system: " << ex.what() << std::endl;
            s_Initialized = true; // Set to true anyway to avoid repeated attempts
            return false;
        }
    }

    void Shutdown() {
        if (!s_Initialized) {
            return;
        }

        LogInfo("Shutting down logging system");
        
#if SPDLOG_AVAILABLE
        if (s_Logger) {
            s_Logger->flush();
            s_Logger.reset();
        }
        
        spdlog::shutdown();
#endif
        s_GuiLogQueue.Clear();
        s_Initialized = false;
    }

    GuiLogQueue& GetGuiLogQueue() {
        return s_GuiLogQueue;
    }

    // Internal helper for logging
    void LogInternal(LogLevel level, const std::string& message) {
        if (!s_Initialized) {
            return;
        }

        // Always add to GUI queue for the console panel
        s_GuiLogQueue.Push(LogMessage(message, level));

#if SPDLOG_AVAILABLE
        if (s_Logger) {
            switch (level) {
                case LogLevel::Trace:    s_Logger->trace(message); break;
                case LogLevel::Debug:    s_Logger->debug(message); break;
                case LogLevel::Info:     s_Logger->info(message); break;
                case LogLevel::Warn:     s_Logger->warn(message); break;
                case LogLevel::Error:    s_Logger->error(message); break;
                case LogLevel::Critical: s_Logger->critical(message); break;
            }
        }
#else
        // Fallback to console output when spdlog isn't available
        const char* levelStr = "";
        switch (level) {
            case LogLevel::Trace:    levelStr = "[TRACE]"; break;
            case LogLevel::Debug:    levelStr = "[DEBUG]"; break;
            case LogLevel::Info:     levelStr = "[INFO]"; break;
            case LogLevel::Warn:     levelStr = "[WARN]"; break;
            case LogLevel::Error:    levelStr = "[ERROR]"; break;
            case LogLevel::Critical: levelStr = "[CRITICAL]"; break;
        }
        std::cout << levelStr << " " << message << std::endl;
#endif
    }

    // Public logging functions
    void LogTrace(const std::string& message) {
        LogInternal(LogLevel::Trace, message);
    }

    void LogDebug(const std::string& message) {
        LogInternal(LogLevel::Debug, message);
    }

    void LogInfo(const std::string& message) {
        LogInternal(LogLevel::Info, message);
    }

    void LogWarn(const std::string& message) {
        LogInternal(LogLevel::Warn, message);
    }

    void LogError(const std::string& message) {
        LogInternal(LogLevel::Error, message);
    }

    void LogCritical(const std::string& message) {
        LogInternal(LogLevel::Critical, message);
    }

} // namespace EngineLogging