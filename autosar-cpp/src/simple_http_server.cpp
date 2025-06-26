/**
 * @file simple_http_server.cpp
 * @brief Simple HTTP Server Implementation
 */

#include "simple_http_server.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace autosar {

SimpleHTTPServer::SimpleHTTPServer(uint16_t port)
    : port_(port), server_socket_(-1)
{
}

SimpleHTTPServer::~SimpleHTTPServer() {
    if (running_.load()) {
        stop();
    }
}

Std_ReturnType SimpleHTTPServer::initialize(
    std::shared_ptr<EngineSWC> engine_swc,
    std::shared_ptr<BodyControlSWC> body_swc,
    std::shared_ptr<RTE> rte
) {
    if (!engine_swc || !body_swc || !rte) {
        logMessage("ERROR", "Cannot initialize SimpleHTTPServer with null components");
        return Std_ReturnType::E_NOT_OK;
    }
    
    engine_swc_ = engine_swc;
    body_swc_ = body_swc;
    rte_ = rte;
    
    logMessage("INFO", "SimpleHTTPServer initialized with AUTOSAR components");
    return Std_ReturnType::E_OK;
}

Std_ReturnType SimpleHTTPServer::start() {
    if (running_.load()) {
        logMessage("WARNING", "SimpleHTTPServer already running");
        return Std_ReturnType::E_NOT_OK;
    }
    
    if (!engine_swc_ || !body_swc_ || !rte_) {
        logMessage("ERROR", "SimpleHTTPServer not initialized");
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        logMessage("ERROR", "Failed to create socket");
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        logMessage("WARNING", "Failed to set socket options");
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logMessage("ERROR", "Failed to bind socket to port " + std::to_string(port_));
        close(server_socket_);
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Listen
    if (listen(server_socket_, 5) < 0) {
        logMessage("ERROR", "Failed to listen on socket");
        close(server_socket_);
        return Std_ReturnType::E_NOT_OK;
    }
    
    running_.store(true);
    server_thread_ = std::thread(&SimpleHTTPServer::serverLoop, this);
    
    logMessage("INFO", "SimpleHTTPServer started on port " + std::to_string(port_));
    logMessage("INFO", "API endpoints available at:");
    logMessage("INFO", "  GET  http://localhost:" + std::to_string(port_) + "/api/vehicle");
    logMessage("INFO", "  GET  http://localhost:" + std::to_string(port_) + "/api/engine");
    logMessage("INFO", "  GET  http://localhost:" + std::to_string(port_) + "/api/body");
    logMessage("INFO", "  POST http://localhost:" + std::to_string(port_) + "/api/command");
    
    return Std_ReturnType::E_OK;
}

Std_ReturnType SimpleHTTPServer::stop() {
    if (!running_.load()) {
        logMessage("WARNING", "SimpleHTTPServer not running");
        return Std_ReturnType::E_NOT_OK;
    }
    
    logMessage("INFO", "Stopping SimpleHTTPServer...");
    running_.store(false);
    
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    logMessage("INFO", "SimpleHTTPServer stopped");
    return Std_ReturnType::E_OK;
}

json SimpleHTTPServer::getServerStats() const {
    return json{
        {"port", port_},
        {"requests_served", requests_served_.load()},
        {"errors_count", errors_count_.load()},
        {"running", running_.load()}
    };
}

void SimpleHTTPServer::serverLoop() {
    while (running_.load()) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_.load()) {  // Only log if not shutting down
                logMessage("WARNING", "Failed to accept client connection");
            }
            continue;
        }
        
        // Handle client in separate thread for better responsiveness
        // Store thread to ensure proper cleanup
        auto worker = std::make_shared<std::thread>([this, client_socket]() {
            handleClient(client_socket);
        });
        worker->detach(); // Allow thread to run independently but avoid resource leak
    }
}

void SimpleHTTPServer::handleClient(int client_socket) {
    // RAII socket guard to ensure socket is always closed
    struct SocketGuard {
        int socket_fd;
        explicit SocketGuard(int fd) : socket_fd(fd) {}
        ~SocketGuard() { if (socket_fd >= 0) close(socket_fd); }
        SocketGuard(const SocketGuard&) = delete;
        SocketGuard& operator=(const SocketGuard&) = delete;
    };
    
    SocketGuard socket_guard(client_socket);
    
    try {
        char buffer[4096];
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        
        if (bytes_read <= 0) {
            return; // Socket will be closed by SocketGuard
        }
        
        // Ensure null termination
        buffer[std::min(bytes_read, static_cast<ssize_t>(sizeof(buffer) - 1))] = '\0';
        std::string request(buffer);
    
        std::string method, path;
        parseHTTPRequest(request, method, path);
        
        logMessage("DEBUG", method + " " + path);
        
        std::string response;
        
        if (path.substr(0, 5) == "/api/") {
            response = handleAPIRequest(method, path);
        } else {
            // Default response for non-API requests
            std::string body = R"(
{
  "message": "AUTOSAR HMI Server",
  "version": "1.0.0",
  "endpoints": [
    "GET /api/vehicle - Get complete vehicle state",
    "GET /api/engine - Get engine data",
    "GET /api/body - Get body control data",
    "POST /api/command - Send command to vehicle"
  ]
}
)";
            response = buildHTTPResponse(200, "application/json", body);
        }
        
        ssize_t bytes_written = write(client_socket, response.c_str(), response.length());
        if (bytes_written < 0) {
            logMessage("WARNING", "Failed to write response to client");
        }
        
        requests_served_++;
    } catch (const std::exception& e) {
        logMessage("ERROR", "Exception in handleClient: " + std::string(e.what()));
        errors_count_++;
    } catch (...) {
        logMessage("ERROR", "Unknown exception in handleClient");
        errors_count_++;
    }
    // Socket automatically closed by SocketGuard destructor
}

std::string SimpleHTTPServer::parseHTTPRequest(const std::string& request, std::string& method, std::string& path) {
    std::istringstream iss(request);
    std::string line;
    std::getline(iss, line);
    
    std::istringstream request_line(line);
    std::string version;
    request_line >> method >> path >> version;
    
    return request;
}

std::string SimpleHTTPServer::buildHTTPResponse(int status_code, const std::string& content_type, const std::string& body) {
    std::string status_text;
    switch (status_code) {
        case 200: status_text = "OK"; break;
        case 400: status_text = "Bad Request"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
        default: status_text = "Unknown"; break;
    }
    
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";  // CORS for web frontend
    response << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response << "Access-Control-Allow-Headers: Content-Type\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    
    return response.str();
}

std::string SimpleHTTPServer::handleAPIRequest(const std::string& method, const std::string& path) {
    try {
        if (method == "OPTIONS") {
            // Handle CORS preflight
            return buildHTTPResponse(200, "text/plain", "");
        } else if (method == "GET" && path == "/api/vehicle") {
            json data = getVehicleState();
            return buildHTTPResponse(200, "application/json", data.dump(2));
        } else if (method == "GET" && path == "/api/engine") {
            json data = getEngineData();
            return buildHTTPResponse(200, "application/json", data.dump(2));
        } else if (method == "GET" && path == "/api/body") {
            json data = getBodyData();
            return buildHTTPResponse(200, "application/json", data.dump(2));
        } else if (method == "POST" && path == "/api/command") {
            // For simplicity, we'll just return success for now
            json response = {
                {"status", "success"},
                {"message", "Command received (not implemented yet)"}
            };
            return buildHTTPResponse(200, "application/json", response.dump(2));
        } else {
            json error = {
                {"error", "Not Found"},
                {"message", "API endpoint not found: " + path}
            };
            return buildHTTPResponse(404, "application/json", error.dump(2));
        }
    } catch (const std::exception& e) {
        errors_count_++;
        json error = {
            {"error", "Internal Server Error"},
            {"message", e.what()}
        };
        return buildHTTPResponse(500, "application/json", error.dump(2));
    }
}

json SimpleHTTPServer::getVehicleState() {
    auto engine_data = engine_swc_->getCurrentEngineData();
    auto door_status = body_swc_->getCurrentDoorStatus();
    auto light_control = body_swc_->getCurrentLightControl();
    
    return json{
        {"type", "VEHICLE_STATE"},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()},
        {"data", {
            {"engine", {
                {"rpm", engine_data.rpm},
                {"temperature", static_cast<int>(engine_data.temperature)},
                {"throttlePosition", static_cast<int>(engine_data.throttle_position)},
                {"running", engine_data.rpm > 0},
                {"oilPressure", 60},  // Simulated
                {"fuelLevel", 75},    // Simulated
                {"speed", static_cast<int>(engine_data.rpm * 0.02)} // Simulated speed based on RPM
            }},
            {"body", {
                {"doors", {
                    {"frontLeft", door_status.front_left_open},
                    {"frontRight", door_status.front_right_open},
                    {"rearLeft", door_status.rear_left_open},
                    {"rearRight", door_status.rear_right_open},
                    {"trunk", false},
                    {"hood", false}
                }},
                {"windows", {
                    {"frontLeft", 100},
                    {"frontRight", 100},
                    {"rearLeft", 100},
                    {"rearRight", 100}
                }},
                {"lights", {
                    {"headlights", light_control.headlights_on ? "LOW_BEAM" : "OFF"},
                    {"turnSignals", light_control.turn_signal_left ? "LEFT" : 
                                   (light_control.turn_signal_right ? "RIGHT" : "OFF")},
                    {"brakelights", false},
                    {"reverselights", false},
                    {"interiorLights", false}
                }}
            }},
            {"ignition", "ON"},
            {"gear", "D"}
        }}
    };
}

json SimpleHTTPServer::getEngineData() {
    auto engine_data = engine_swc_->getCurrentEngineData();
    
    return json{
        {"rpm", engine_data.rpm},
        {"temperature", static_cast<int>(engine_data.temperature)},
        {"throttlePosition", static_cast<int>(engine_data.throttle_position)},
        {"running", engine_data.rpm > 0},
        {"oilPressure", 60},
        {"fuelLevel", 75},
        {"speed", static_cast<int>(engine_data.rpm * 0.02)}
    };
}

json SimpleHTTPServer::getBodyData() {
    auto door_status = body_swc_->getCurrentDoorStatus();
    auto light_control = body_swc_->getCurrentLightControl();
    
    return json{
        {"doors", {
            {"frontLeft", door_status.front_left_open},
            {"frontRight", door_status.front_right_open},
            {"rearLeft", door_status.rear_left_open},
            {"rearRight", door_status.rear_right_open},
            {"trunk", false},
            {"hood", false}
        }},
        {"windows", {
            {"frontLeft", 100},
            {"frontRight", 100},
            {"rearLeft", 100},
            {"rearRight", 100}
        }},
        {"lights", {
            {"headlights", light_control.headlights_on ? "LOW_BEAM" : "OFF"},
            {"turnSignals", light_control.turn_signal_left ? "LEFT" : 
                           (light_control.turn_signal_right ? "RIGHT" : "OFF")},
            {"brakelights", false},
            {"reverselights", false},
            {"interiorLights", false}
        }}
    };
}

void SimpleHTTPServer::logMessage(const std::string& level, const std::string& message) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") 
              << "] [SimpleHTTPServer] [" << level << "] " << message << std::endl;
}

} // namespace autosar