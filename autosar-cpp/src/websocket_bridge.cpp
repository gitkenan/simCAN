/**
 * @file websocket_bridge.cpp
 * @brief WebSocket Bridge Implementation
 */

#include "websocket_bridge.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace autosar {

WebSocketBridge::WebSocketBridge(uint16_t port, double update_rate_hz)
    : port_(port)
    , update_rate_hz_(update_rate_hz)
    , update_interval_(static_cast<int>(1000.0 / update_rate_hz))
{
    // Configure WebSocket server
    server_.set_access_channels(websocketpp::log::alevel::all);
    server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
    server_.set_error_channels(websocketpp::log::elevel::all);
    
    server_.init_asio();
    server_.set_reuse_addr(true);
    
    // Set handlers
    server_.set_open_handler([this](connection_hdl hdl) { onOpen(hdl); });
    server_.set_close_handler([this](connection_hdl hdl) { onClose(hdl); });
    server_.set_message_handler([this](connection_hdl hdl, WebSocketServer::message_ptr msg) {
        onMessage(hdl, msg);
    });
}

WebSocketBridge::~WebSocketBridge() {
    if (running_.load()) {
        stop();
    }
}

Std_ReturnType WebSocketBridge::initialize(
    std::shared_ptr<EngineSWC> engine_swc,
    std::shared_ptr<BodyControlSWC> body_swc,
    std::shared_ptr<RTE> rte
) {
    if (!engine_swc || !body_swc || !rte) {
        logMessage("ERROR", "Cannot initialize WebSocketBridge with null components");
        return Std_ReturnType::E_NOT_OK;
    }
    
    engine_swc_ = engine_swc;
    body_swc_ = body_swc;
    rte_ = rte;
    
    logMessage("INFO", "WebSocketBridge initialized with AUTOSAR components");
    return Std_ReturnType::E_OK;
}

Std_ReturnType WebSocketBridge::start() {
    if (running_.load()) {
        logMessage("WARNING", "WebSocketBridge already running");
        return Std_ReturnType::E_NOT_OK;
    }
    
    if (!engine_swc_ || !body_swc_ || !rte_) {
        logMessage("ERROR", "WebSocketBridge not initialized - missing AUTOSAR components");
        return Std_ReturnType::E_NOT_OK;
    }
    
    try {
        // Listen on specified port
        server_.listen(port_);
        server_.start_accept();
        
        running_.store(true);
        start_time_ = std::chrono::steady_clock::now();
        
        // Start server thread
        server_thread_ = std::thread(&WebSocketBridge::serverLoop, this);
        
        // Start broadcast thread
        broadcast_thread_ = std::thread(&WebSocketBridge::broadcastLoop, this);
        
        logMessage("INFO", "WebSocketBridge started on port " + std::to_string(port_));
        return Std_ReturnType::E_OK;
        
    } catch (const std::exception& e) {
        logMessage("ERROR", "Failed to start WebSocketBridge: " + std::string(e.what()));
        running_.store(false);
        return Std_ReturnType::E_NOT_OK;
    }
}

Std_ReturnType WebSocketBridge::stop() {
    if (!running_.load()) {
        logMessage("WARNING", "WebSocketBridge not running");
        return Std_ReturnType::E_NOT_OK;
    }
    
    logMessage("INFO", "Stopping WebSocketBridge...");
    running_.store(false);
    
    try {
        // Stop server
        server_.stop();
        
        // Join threads
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        
        if (broadcast_thread_.joinable()) {
            broadcast_thread_.join();
        }
        
        // Clear connections
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            active_connections_.clear();
        }
        
        logMessage("INFO", "WebSocketBridge stopped successfully");
        return Std_ReturnType::E_OK;
        
    } catch (const std::exception& e) {
        logMessage("ERROR", "Error stopping WebSocketBridge: " + std::string(e.what()));
        return Std_ReturnType::E_NOT_OK;
    }
}

size_t WebSocketBridge::getConnectedClients() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    return active_connections_.size();
}

json WebSocketBridge::getServerStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        now - start_time_
    ).count();
    
    return json{
        {"uptime_seconds", uptime_seconds},
        {"connected_clients", getConnectedClients()},
        {"messages_sent", messages_sent_},
        {"messages_received", messages_received_},
        {"errors_count", errors_count_},
        {"update_rate_hz", update_rate_hz_},
        {"port", port_}
    };
}

void WebSocketBridge::serverLoop() {
    try {
        server_.run();
    } catch (const std::exception& e) {
        logMessage("ERROR", "Server loop exception: " + std::string(e.what()));
    }
}

void WebSocketBridge::broadcastLoop() {
    while (running_.load()) {
        try {
            // Get current vehicle state and broadcast to all clients
            json vehicle_state = serializeVehicleState();
            broadcastMessage(vehicle_state);
            
            std::this_thread::sleep_for(update_interval_);
            
        } catch (const std::exception& e) {
            logMessage("ERROR", "Broadcast loop exception: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}

void WebSocketBridge::onOpen(connection_hdl hdl) {
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        active_connections_.push_back(hdl);
    }
    
    logMessage("INFO", "Client connected. Total clients: " + 
               std::to_string(getConnectedClients()));
    
    // Send initial system status
    json status_message = {
        {"type", "SYSTEM_STATUS"},
        {"status", "CONNECTED"},
        {"message", "Connected to AUTOSAR vehicle simulation"},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()}
    };
    
    try {
        server_.send(hdl, status_message.dump(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        logMessage("ERROR", "Failed to send welcome message: " + std::string(e.what()));
    }
}

void WebSocketBridge::onClose(connection_hdl hdl) {
    removeConnection(hdl);
    logMessage("INFO", "Client disconnected. Total clients: " + 
               std::to_string(getConnectedClients()));
}

void WebSocketBridge::onMessage(connection_hdl hdl, WebSocketServer::message_ptr msg) {
    try {
        json command = json::parse(msg->get_payload());
        
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            messages_received_++;
        }
        
        if (processUserCommand(command)) {
            logMessage("DEBUG", "Processed command: " + command.dump());
        } else {
            sendErrorMessage(hdl, "Invalid or unsupported command");
        }
        
    } catch (const json::parse_error& e) {
        sendErrorMessage(hdl, "Invalid JSON format: " + std::string(e.what()));
    } catch (const std::exception& e) {
        sendErrorMessage(hdl, "Command processing error: " + std::string(e.what()));
    }
}

json WebSocketBridge::serializeVehicleState() const {
    auto engine_data = engine_swc_->getCurrentEngineData();
    auto door_status = body_swc_->getCurrentDoorStatus();
    auto light_control = body_swc_->getCurrentLightControl();
    
    return json{
        {"type", "VEHICLE_STATE"},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()},
        {"data", {
            {"engine", serializeEngineData(engine_data)},
            {"body", {
                {"doors", serializeDoorStatus(door_status)},
                {"windows", {
                    {"frontLeft", 100},
                    {"frontRight", 100},
                    {"rearLeft", 100},
                    {"rearRight", 100}
                }},
                {"lights", serializeLightControl(light_control)}
            }},
            {"ignition", "ON"},
            {"gear", "D"},
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()}
        }}
    };
}

json WebSocketBridge::serializeEngineData(const EngineData& data) const {
    return json{
        {"rpm", data.rpm},
        {"temperature", static_cast<int>(data.temperature)},
        {"throttlePosition", static_cast<int>(data.throttle_position)},
        {"running", data.rpm > 0},
        {"oilPressure", 60},  // Simulated value
        {"fuelLevel", 75}     // Simulated value
    };
}

json WebSocketBridge::serializeDoorStatus(const DoorStatus& status) const {
    return json{
        {"frontLeft", status.front_left_open},
        {"frontRight", status.front_right_open},
        {"rearLeft", status.rear_left_open},
        {"rearRight", status.rear_right_open},
        {"trunk", false},     // Not in current DoorStatus
        {"hood", false}       // Not in current DoorStatus
    };
}

json WebSocketBridge::serializeLightControl(const LightControl& lights) const {
    std::string headlight_state = "OFF";
    if (lights.headlights_on) {
        headlight_state = "LOW_BEAM";
    }
    
    std::string turn_signal_state = "OFF";
    if (lights.turn_signal_left) {
        turn_signal_state = "LEFT";
    } else if (lights.turn_signal_right) {
        turn_signal_state = "RIGHT";
    }
    
    return json{
        {"headlights", headlight_state},
        {"turnSignals", turn_signal_state},
        {"brakelights", false},        // Not in current LightControl
        {"reverselights", false},      // Not in current LightControl
        {"interiorLights", false}      // Not in current LightControl
    };
}

bool WebSocketBridge::processUserCommand(const json& command) {
    try {
        if (!command.contains("type") || command["type"] != "USER_COMMAND") {
            return false;
        }
        
        if (!command.contains("component") || !command.contains("action")) {
            return false;
        }
        
        std::string component = command["component"];
        
        if (component == "engine") {
            return processEngineCommand(command);
        } else if (component == "body") {
            return processBodyCommand(command);
        } else if (component == "ignition") {
            return processIgnitionCommand(command);
        } else if (component == "transmission") {
            return processTransmissionCommand(command);
        }
        
        return false;
        
    } catch (const std::exception& e) {
        logMessage("ERROR", "Error processing command: " + std::string(e.what()));
        return false;
    }
}

bool WebSocketBridge::processEngineCommand(const json& command) {
    std::string action = command["action"];
    
    if (action == "setRPM" && command.contains("parameters") && 
        command["parameters"].contains("value")) {
        // Note: Current EngineSWC doesn't have setRPM method
        // This would need to be added to support HMI control
        logMessage("INFO", "Engine RPM command received (not implemented)");
        return true;
    }
    
    return false;
}

bool WebSocketBridge::processBodyCommand(const json& command) {
    std::string action = command["action"];
    
    if (action == "toggleDoor" && command.contains("parameters") && 
        command["parameters"].contains("door")) {
        
        std::string door = command["parameters"]["door"];
        bool new_state = true; // Toggle logic would need current state
        
        if (door == "frontLeft") {
            body_swc_->setDoorOpen(0, new_state);
        } else if (door == "frontRight") {
            body_swc_->setDoorOpen(1, new_state);
        } else if (door == "rearLeft") {
            body_swc_->setDoorOpen(2, new_state);
        } else if (door == "rearRight") {
            body_swc_->setDoorOpen(3, new_state);
        }
        
        logMessage("INFO", "Door command processed: " + door);
        return true;
        
    } else if (action == "setLight" && command.contains("parameters") && 
               command["parameters"].contains("lightType") && 
               command["parameters"].contains("value")) {
        
        std::string lightType = command["parameters"]["lightType"];
        
        if (lightType == "headlights") {
            std::string value = command["parameters"]["value"];
            LightControl lights = body_swc_->getCurrentLightControl();
            lights.headlights_on = (value != "OFF");
            body_swc_->setLightControl(lights);
            
            logMessage("INFO", "Headlight command processed: " + value);
            return true;
        }
    }
    
    return false;
}

bool WebSocketBridge::processIgnitionCommand(const json& command) {
    // Ignition control would need to be added to the system
    logMessage("INFO", "Ignition command received (not implemented)");
    return true;
}

bool WebSocketBridge::processTransmissionCommand(const json& command) {
    // Transmission control would need to be added to the system
    logMessage("INFO", "Transmission command received (not implemented)");
    return true;
}

void WebSocketBridge::broadcastMessage(const json& message) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    std::string message_str = message.dump();
    
    auto it = active_connections_.begin();
    while (it != active_connections_.end()) {
        try {
            server_.send(*it, message_str, websocketpp::frame::opcode::text);
            ++it;
        } catch (const std::exception& e) {
            logMessage("WARNING", "Failed to send to client, removing connection");
            it = active_connections_.erase(it);
        }
    }
    
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        messages_sent_++;
    }
}

void WebSocketBridge::sendErrorMessage(connection_hdl hdl, const std::string& error) {
    json error_message = {
        {"type", "SYSTEM_STATUS"},
        {"status", "ERROR"},
        {"message", error},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()}
    };
    
    try {
        server_.send(hdl, error_message.dump(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        logMessage("ERROR", "Failed to send error message: " + std::string(e.what()));
    }
    
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        errors_count_++;
    }
}

void WebSocketBridge::logMessage(const std::string& level, const std::string& message) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
              << "] [WebSocketBridge] [" << level << "] " << message << std::endl;
}

void WebSocketBridge::removeConnection(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    active_connections_.erase(
        std::remove_if(active_connections_.begin(), active_connections_.end(),
            [&](const connection_hdl& conn) {
                return !conn.owner_before(hdl) && !hdl.owner_before(conn);
            }),
        active_connections_.end()
    );
}

// JSON serialization implementations
void to_json(json& j, const EngineData& data) {
    j = json{
        {"rpm", data.rpm},
        {"temperature", static_cast<int>(data.temperature)},
        {"throttlePosition", static_cast<int>(data.throttle_position)},
        {"running", data.rpm > 0}
    };
}

void to_json(json& j, const DoorStatus& status) {
    j = json{
        {"frontLeft", status.front_left_open},
        {"frontRight", status.front_right_open},
        {"rearLeft", status.rear_left_open},
        {"rearRight", status.rear_right_open}
    };
}

void to_json(json& j, const LightControl& lights) {
    j = json{
        {"headlights", lights.headlights_on},
        {"turnSignalLeft", lights.turn_signal_left},
        {"turnSignalRight", lights.turn_signal_right}
    };
}

} // namespace autosar