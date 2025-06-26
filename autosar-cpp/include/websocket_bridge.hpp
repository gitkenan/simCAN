/**
 * @file websocket_bridge.hpp
 * @brief WebSocket Bridge for AUTOSAR HMI Communication
 * 
 * This component bridges the AUTOSAR C++ simulation with web-based HMI frontends
 * via WebSocket communication. It provides real-time bidirectional data exchange
 * between the automotive systems and modern web interfaces.
 * 
 * Features:
 * - Real-time vehicle state broadcasting
 * - Command reception from HMI clients
 * - JSON serialization of AUTOSAR data structures
 * - Automotive-grade error handling and logging
 */

#pragma once

#include "autosar_types.hpp"
#include "engine_swc.hpp"
#include "body_swc.hpp"
#include "rte.hpp"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>
#include <string>

namespace autosar {

using json = nlohmann::json;
using websocketpp::connection_hdl;

/**
 * @brief WebSocket Bridge for AUTOSAR-HMI Communication
 * 
 * This class provides a WebSocket server that enables real-time communication
 * between AUTOSAR software components and web-based human-machine interfaces.
 * It handles JSON serialization of vehicle data and command processing.
 */
class WebSocketBridge {
public:
    /**
     * @brief Message types for WebSocket communication
     */
    enum class MessageType {
        VEHICLE_STATE,
        USER_COMMAND,
        SYSTEM_STATUS,
        ERROR
    };

    /**
     * @brief Command components that can receive user commands
     */
    enum class CommandComponent {
        ENGINE,
        BODY,
        IGNITION,
        TRANSMISSION
    };

    /**
     * @brief Constructor
     * @param port WebSocket server port (default: 8080)
     * @param update_rate_hz Rate for broadcasting vehicle state (default: 10Hz)
     */
    explicit WebSocketBridge(uint16_t port = 8080, double update_rate_hz = 10.0);

    /**
     * @brief Destructor - ensures clean shutdown
     */
    ~WebSocketBridge();

    /**
     * @brief Initialize the WebSocket bridge with AUTOSAR components
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
     * @brief Start the WebSocket server
     * @return E_OK on success, E_NOT_OK on failure
     */
    Std_ReturnType start();

    /**
     * @brief Stop the WebSocket server
     * @return E_OK on success, E_NOT_OK on failure
     */
    Std_ReturnType stop();

    /**
     * @brief Check if the server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_.load(); }

    /**
     * @brief Get the number of connected clients
     * @return Number of active WebSocket connections
     */
    size_t getConnectedClients() const;

    /**
     * @brief Get server statistics
     * @return JSON object with server statistics
     */
    json getServerStats() const;

private:
    // WebSocket server type
    using WebSocketServer = websocketpp::server<websocketpp::config::asio>;

    // Server configuration
    uint16_t port_;
    double update_rate_hz_;
    std::chrono::milliseconds update_interval_;

    // WebSocket server
    WebSocketServer server_;
    std::thread server_thread_;
    std::thread broadcast_thread_;
    std::atomic<bool> running_{false};

    // AUTOSAR components
    std::shared_ptr<EngineSWC> engine_swc_;
    std::shared_ptr<BodyControlSWC> body_swc_;
    std::shared_ptr<RTE> rte_;

    // Connection management
    mutable std::mutex connections_mutex_;
    std::vector<connection_hdl> active_connections_;

    // Statistics
    mutable std::mutex stats_mutex_;
    uint64_t messages_sent_{0};
    uint64_t messages_received_{0};
    uint64_t errors_count_{0};
    std::chrono::steady_clock::time_point start_time_;

    // Private methods
    void serverLoop();
    void broadcastLoop();
    
    // WebSocket event handlers
    void onOpen(connection_hdl hdl);
    void onClose(connection_hdl hdl);
    void onMessage(connection_hdl hdl, WebSocketServer::message_ptr msg);
    
    // Data serialization
    json serializeVehicleState() const;
    json serializeEngineData(const EngineData& data) const;
    json serializeDoorStatus(const DoorStatus& status) const;
    json serializeLightControl(const LightControl& lights) const;
    
    // Command processing
    bool processUserCommand(const json& command);
    bool processEngineCommand(const json& command);
    bool processBodyCommand(const json& command);
    bool processIgnitionCommand(const json& command);
    bool processTransmissionCommand(const json& command);
    
    // Utility methods
    void broadcastMessage(const json& message);
    void sendErrorMessage(connection_hdl hdl, const std::string& error);
    void logMessage(const std::string& level, const std::string& message) const;
    
    // Helper to safely remove connections
    void removeConnection(connection_hdl hdl);
};

/**
 * @brief JSON serialization support for AUTOSAR types
 */
void to_json(json& j, const EngineData& data);
void to_json(json& j, const DoorStatus& status);
void to_json(json& j, const LightControl& lights);

} // namespace autosar