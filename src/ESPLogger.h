/**
 * @file ESPLogger.h
 * @brief Thread-safe logging system for ESP32 and similar embedded systems.
 * 
 * This logger implements a circular buffer for storing log messages,
 * supports multiple log levels, and provides both callback and observer
 * patterns for flexible log handling.
 * 
 * @todo Implement configurable buffer sizes
 * @todo Add timestamp information to log entries
 * @todo Implement log rotation or file-based logging for persistence
 * @todo Use compile-time configuration for system-specific optimizations
 * @todo Consider using a more type-safe formatting library
 * @todo Optimize memory usage for callbacks and observers
 * @todo Add utility methods for logging exceptions and stack traces
 */

#ifndef ESP_LOGGER_H
#define ESP_LOGGER_H

#include <array>
#include <string>
#include <string_view>
#include <functional>
#include <mutex>
#include <atomic>
#include <vector>
#include <ArduinoJson.h>

#ifdef ENABLE_SERIAL_PRINT
#include <Arduino.h>
#endif

/**
 * @class Logger
 * @brief Main logger class implementing a thread-safe circular buffer for log messages.
 */
class Logger {
public:
    /**
     * @enum Level
     * @brief Enumeration of log severity levels.
     */
    enum class Level { DEBUG, INFO, WARNING, ERROR };

    static constexpr size_t MAX_LOGS = 100; ///< Maximum number of logs in the circular buffer
    static constexpr size_t LOG_SIZE = 156; ///< Maximum size of a log message
    static constexpr size_t TAG_SIZE = 20;  ///< Maximum size of a log tag
    static constexpr std::string_view DEFAULT_TAG = "DEFAULT"; ///< Default tag for logs
    static constexpr std::string_view OVERFLOW_MSG = " [LOG OVERFLOW]"; ///< Message appended when a log message is truncated

    /**
     * @brief Get the singleton instance of the Logger.
     * @return Reference to the Logger instance.
     */
    static Logger& instance();

    /**
     * @brief Set a callback function to be called for each log entry.
     * @param cb Callback function taking tag, level, and message as parameters.
     */
    void setCallback(std::function<void(std::string_view, Level, std::string_view)> cb);

    /**
     * @brief Add an observer function to be called for each log entry.
     * @param observer Observer function taking tag, level, and message as parameters.
     */
    void addLogObserver(std::function<void(std::string_view, Level, std::string_view)> observer);

    /**
     * @brief Set the minimum log level to be processed.
     * @param level Minimum log level.
     */
    void setFilterLevel(Level level);

    /**
     * @brief Log a message with formatting.
     * @param tag Tag for the log entry.
     * @param level Severity level of the log.
     * @param format Format string for the log message.
     * @param args Arguments to be formatted into the log message.
     */
    template<typename... Args>
    void log(std::string_view tag, Level level, const char* format, Args&&... args) {
        if (level >= filterLevel.load(std::memory_order_relaxed)) {
            char message[LOG_SIZE];
            snprintf(message, sizeof(message), format, std::forward<Args>(args)...);
            addLog(tag, level, message);
        }
    }

    /**
     * @brief Overload, Log a message without formatting.
     * @param tag Tag for the log entry.
     * @param level Severity level of the log.
     * @param message Content of the log message.
     */
    void log(std::string_view tag, Level level, const char* message);

    
    /**
     * @struct LogEntry
     * @brief Structure representing a single log entry.
     */
    struct LogEntry {
        char tag[TAG_SIZE];  ///< Tag of the log entry
        Level level;         ///< Severity level of the log entry
        char message[LOG_SIZE]; ///< Content of the log message
    };

    /**
     * @brief Retrieve and remove the next log entry from the buffer.
     * @param entry Reference to a LogEntry structure to be filled.
     * @return true if a log entry was retrieved, false if the buffer is empty.
     */
    bool getNextLog(LogEntry& entry);

    /**
     * @brief Retrieve and remove the next log entry as a JSON string.
     * @return JSON string representation of the next log entry, or empty string if buffer is empty.
     */
    String getNextLogJson();

    /**
     * @brief View a log entry without removing it from the buffer.
     * @param entry Reference to a LogEntry structure to be filled.
     * @param offset Offset from the oldest log entry (default is 0).
     * @return true if a log entry was retrieved, false if the offset is out of range.
     */
    bool peekNextLog(LogEntry& entry, size_t offset = 0);

    /**
     * @brief View a log entry as a JSON string without removing it from the buffer.
     * @param offset Offset from the oldest log entry (default is 0).
     * @return JSON string representation of the log entry, or empty string if offset is out of range.
     */
    String peekNextLogJson(size_t offset = 0);

    /**
     * @brief Get the number of valid log entries in the buffer.
     * @return Number of valid log entries.
     */
    size_t getValidLogCount() const;

    /**
     * @brief Get the total number of log entries added since startup.
     * @return Total number of log entries.
     */
    size_t getLogCount() const;

private:
    std::array<LogEntry, MAX_LOGS> buffer; ///< Circular buffer for storing log entries
    std::atomic<size_t> head{0};  ///< Index of the next position to write a log entry
    std::atomic<size_t> tail{0};  ///< Index of the oldest log entry
    std::atomic<size_t> count{0}; ///< Number of log entries currently in the buffer
    std::atomic<size_t> firstLogIndex{0}; ///< Index of the first valid log entry
    std::function<void(std::string_view, Level, std::string_view)> callback; ///< Callback function for log entries
    std::vector<std::function<void(std::string_view, Level, std::string_view)>> observers; ///< List of observer functions
    mutable std::mutex logMutex; ///< Mutex for thread-safe operations
    std::atomic<Level> filterLevel{Level::DEBUG}; ///< Minimum log level to process

    /**
     * @brief Add a log entry to the buffer.
     * @param tag Tag for the log entry.
     * @param level Severity level of the log.
     * @param message Content of the log message.
     */
    void addLog(std::string_view tag, Level level, const char* message);

    Logger(); ///< Private constructor for singleton pattern
    Logger(const Logger&) = delete; ///< Deleted copy constructor
    Logger& operator=(const Logger&) = delete; ///< Deleted assignment operator
};

/**
 * @typedef LogLevel
 * @brief Alias for Logger::Level for easier use.
 */
using LogLevel = Logger::Level;

#endif // ESP_LOGGER_H