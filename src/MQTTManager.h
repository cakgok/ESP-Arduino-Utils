/**
 * @file ESPMQTTManager.h
 * @brief Thread-safe MQTT client manager for ESP32 with TLS support.
 *
 * This class provides a robust MQTT client implementation for ESP32 devices,
 * featuring TLS support, asynchronous operation, automatic reconnection,
 * and a publish buffer for handling network instabilities.
 *
 * @todo Implement support for different MQTT QoS levels
 * @todo Add support for MQTT will messages
 * @todo Consider adding message persistence across power cycles or crashes
 * @todo Evaluate and improve scalability for large number of topics or high message throughput
 * @todo Enhance error handling and recovery mechanisms
 */

#ifndef ESP_MQTT_MANAGER_H
#define ESP_MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "ESPLogger.h"
#include <vector>
#include <queue>
#include <utility>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

/**
 * @class ESPMQTTManager
 * @brief Wrapper class for PubSubClient to simplify MQTT operations.
 */
class ESPMQTTManager {
public:
    /**
     * @enum AuthMode
     * @brief Enumeration of authentication modes for MQTT connection.
     */
    enum class AuthMode { 
        TLS_CERT_AUTH,      /**< TLS authentication using certificates */
        TLS_USER_PASS_AUTH  /**< TLS authentication using username and password */
    };

    /**
     * @struct Config
     * @brief Configuration structure for the MQTT manager.
     */
    struct Config {
        const char* server;           /**< MQTT broker server address */
        int port;                     /**< MQTT broker port */
        const char* username;         /**< Username for authentication */
        const char* password;         /**< Password for authentication */
        const char* rootCA;           /**< Root CA certificate for TLS */
        const char* clientCert;       /**< Client certificate for TLS */
        const char* clientKey;        /**< Client key for TLS */
        const char* clientID;         /**< Client ID for MQTT connection */
        uint32_t reconnectInterval = 5000;   /**< Interval between reconnection attempts in ms */
        uint32_t publishTimeout = 1000;      /**< Timeout for publish operations in ms */
        uint16_t maxRetries = 5;             /**< Maximum number of reconnection attempts */
        AuthMode authMode = AuthMode::TLS_USER_PASS_AUTH;  /**< Authentication mode */
        size_t publishBufferSize = 5;        /**< Size of the publish buffer queue */
    };

    /**
     * @struct PublishItem
     * @brief Structure representing an item in the publish buffer.
     */
    struct PublishItem {
        String topic;    /**< The topic to publish to */
        String payload;  /**< The message payload */
        bool retained;   /**< Whether the message should be retained by the broker */
    };

    /**
     * @brief Constructor for the ESPMQTTManager.
     * @param config The configuration for the MQTT manager.
     */
    ESPMQTTManager(const Config& config);

    /**
     * @brief Destructor for the ESPMQTTManager.
     */
    ~ESPMQTTManager();

    /**
     * @brief Initializes the MQTT manager and starts the background task.
     * @return true if initialization was successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Stops the MQTT manager and cleans up resources.
     */
    void stop();

    /**
     * @brief Publishes a message to a specified topic.
     * @param topic The topic to publish to.
     * @param payload The message payload.
     * @param retained Whether the message should be retained by the broker.
     * @return true if the publish operation was successful or queued, false otherwise.
     */
    bool publish(const char* topic, const char* payload, bool retained = false);

    /**
     * @brief Subscribes to a specified topic.
     * @param topic The topic to subscribe to.
     * @param qos The Quality of Service level for the subscription.
     * @return true if the subscribe operation was successful, false otherwise.
     */
    bool subscribe(const char* topic, uint8_t qos = 0);

    /**
     * @brief Checks if the client is currently connected to the MQTT broker.
     * @return true if connected, false otherwise.
     */
    bool isConnected();

    /**
     * @brief Sets the callback function for incoming messages.
     * @param callback The function to be called when a message is received.
     */
    void setCallback(MQTT_CALLBACK_SIGNATURE);

    /**
     * @brief Sets the authentication mode for the MQTT connection.
     * @param mode The authentication mode to use.
     */
    void setAuthMode(AuthMode mode);

    /**
     * @brief Gets the underlying PubSubClient instance.
     * @return Reference to the PubSubClient instance.
     */
    PubSubClient& getClient();

private:
    static void taskWrapper(void* pvParameters);
    void task();
    bool connect();
    void disconnect();
    void resubscribe();
    void setupTLS();
    String getClientId() const;
    void processPublishBuffer();

    Logger& logger;
    Config config;
    WiFiClientSecure espClient;
    PubSubClient mqttClient;
    TaskHandle_t taskHandle;
    SemaphoreHandle_t mqttMutex;
    std::vector<std::pair<String, uint8_t>> subscriptions;
    volatile bool running;
    uint16_t retryCount;
    QueueHandle_t publishBuffer;
};

#endif // ESP_MQTT_MANAGER_H