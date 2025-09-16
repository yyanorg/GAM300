#include "pch.h"
#include "Logging.hpp"

#include "spdlog/spdlog.h" 
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/pattern_formatter.h"

#include <filesystem>

namespace EngineLogging {
       
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
            assert(!message.empty() && "Log message should not be empty");
            
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

    static GuiLogQueue s_GuiLogQueue;
    static bool s_Initialized = false;

    // GuiLogQueue implementation
    void GuiLogQueue::Push(const LogMessage& message) {
        assert(!message.text.empty() && "Log message text cannot be empty");
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        // Remove old messages if queue is full
        while (m_Queue.size() >= MAX_QUEUE_SIZE) {
            assert(!m_Queue.empty() && "Queue should not be empty when size >= MAX_QUEUE_SIZE");
            m_Queue.pop();
        }
        
        m_Queue.push(message);
        assert(m_Queue.size() <= MAX_QUEUE_SIZE && "Queue size should not exceed maximum");
    }

    bool GuiLogQueue::TryPop(LogMessage& message) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Queue.empty()) {
            return false;
        }
        
        message = m_Queue.front();
        assert(!message.text.empty() && "Popped message should not be empty");
        m_Queue.pop();
        return true;
    }

    void GuiLogQueue::Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        std::queue<LogMessage> empty;
        m_Queue.swap(empty);
        assert(m_Queue.empty() && "Queue should be empty after clear");
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
            assert(std::filesystem::exists("logs") && "Logs directory should exist after creation");

            // Create sinks
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            assert(console_sink != nullptr && "Console sink creation failed");
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/engine.log", true);
            assert(file_sink != nullptr && "File sink creation failed");
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

            auto gui_sink = std::make_shared<GuiSink>(s_GuiLogQueue);
            assert(gui_sink != nullptr && "GUI sink creation failed");
            gui_sink->set_level(spdlog::level::trace);
            gui_sink->set_pattern("%v");

            // Create logger with multiple sinks
            std::vector<spdlog::sink_ptr> sinks{ console_sink, file_sink, gui_sink };
            assert(!sinks.empty() && "Sinks vector should not be empty");
            
            s_Logger = std::make_shared<spdlog::logger>("engine", sinks.begin(), sinks.end());
            assert(s_Logger != nullptr && "Logger creation failed");
            
            s_Logger->set_level(spdlog::level::trace);
            s_Logger->flush_on(spdlog::level::warn);

            // Register as default logger
            spdlog::set_default_logger(s_Logger);

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
        
        if (s_Logger) {
            s_Logger->flush();
            s_Logger.reset();
        }
        
        spdlog::shutdown();
        s_GuiLogQueue.Clear();
        s_Initialized = false;
    }

    GuiLogQueue& GetGuiLogQueue() {
        assert(s_Initialized && "Logging system must be initialized before accessing GUI queue");
        return s_GuiLogQueue;
    }

    // Internal helper for logging
    void LogInternal(LogLevel level, const std::string& message) {
        assert(!message.empty() && "Log message cannot be empty");
        
        if (!s_Initialized) {
            return;
        }

        assert(s_Logger != nullptr && "Logger should be valid when initialized");
        
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

}