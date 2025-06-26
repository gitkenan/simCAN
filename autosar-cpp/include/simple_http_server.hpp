/**
 * @file simple_http_server.hpp
 * @brief Simple HTTP Server for AUTOSAR HMI Communication
 * 
 * A minimal HTTP server implementation that provides:
 * - REST API endpoints for vehicle data
 * - CORS support for web frontend integration
 * - JSON response formatting
 * - Basic error handling
 */

#pragma once

#include "autosar_types.hpp"
#include "engine_swc.hpp"
#include "body_swc.hpp"
#include "rte.hpp"

#include <nlohmann/json.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <string>
#include <map>

namespace autosar {

using json = nlohmann::json;

/**
 * @brief Simple HTTP Server for AUTOSAR-HMI Communication
 * 
 * This class provides a lightweight HTTP server that enables communication
 * between AUTOSAR software components and web-based interfaces through
 * REST API endpoints.
 */
class SimpleHTTPServer {
public:
    /**
     * @brief Constructor
     * @param port HTTP server port (default: 8080)
     */
    explicit SimpleHTTPServer(uint16_t port = 8080);

    /**
     * @brief Destructor - ensures clean shutdown
     */
    ~SimpleHTTPServer();

    /**
     * @brief Initialize the HTTP server with AUTOSAR components
     * @param engine_swc Engine Software Component
     * @param body_swc Body Control Software Component
     * @param rte Runtime Environment
     * @return E_OK on success, E_NOT_OK on failure
     */
    Std_ReturnType initialize(
        std::shared_ptr<EngineSWC> engine_swc,
        std::shared_ptr<BodyControlSWC> body_swc,
        std::shared_ptr<RTE> rte
    );

    /**
     * @brief Start the HTTP server
     * @return E_OK on success, E_NOT_OK on failure
     */
    Std_ReturnType start();

    /**
     * @brief Stop the HTTP server
     * @return E_OK on success, E_NOT_OK on failure
     */
    Std_ReturnType stop();

    /**
     * @brief Check if the server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief Get server statistics
     * @return JSON object with server statistics
     */
    json getServerStats() const;

private:
    // Server configuration
    uint16_t port_;
    int server_socket_;
    std::thread server_thread_;
    std::atomic<bool> running_{false};

    // AUTOSAR components
    std::shared_ptr<EngineSWC> engine_swc_;
    std::shared_ptr<BodyControlSWC> body_swc_;
    std::shared_ptr<RTE> rte_;

    // Statistics
    std::atomic<uint64_t> requests_served_{0};
    std::atomic<uint64_t> errors_count_{0};

    // Private methods
    void serverLoop();
    void handleClient(int client_socket);
    
    // HTTP handling
    std::string parseHTTPRequest(const std::string& request, std::string& method, std::string& path);
    std::string buildHTTPResponse(int status_code, const std::string& content_type, const std::string& body);
    std::string handleAPIRequest(const std::string& method, const std::string& path);
    
    // API endpoints
    json getVehicleState();
    json getEngineData();
    json getBodyData();
    std::string handleCommand(const std::string& body);
    
    // Utility methods
    void logMessage(const std::string& level, const std::string& message) const;
};

} // namespace autosar