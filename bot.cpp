#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <atomic>
#include <condition_variable>
#include <regex>
#include <algorithm>
#include <ctime>

#include <curl/curl.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

using namespace std::chrono;

// ==================== CONFIGURATION ====================
std::string API_ID = "27896193";
std::string API_HASH = "38a5463cb8bf980d4519fba0ced298c2";
int64_t MAIN_OWNER = 5286579067;
int SEND_DELAY_MS = 100;
std::string AUTO_JOIN_LINK = "https://t.me/TheVillainActive";

struct Session {
    int index;
    std::string session_string;
    int64_t owner;
    int64_t user_id;
    std::string phone;
    bool active;
};

std::vector<Session> SESSIONS;
std::vector<int64_t> SUDO_USERS;

// ==================== ABUSE MESSAGES ====================
std::vector<std::string> ABUSE_LIST = {
    "𝗧𝗘𝗥𝗜 𝗠𝗔𝗔 𝗞𝗜 𝗖𝗛𝗨𝗧 𝗠𝗘𝗜𝗡 𝟭𝟬𝟬𝟬 𝗟𝗔𝗧𝗛𝗜 𝗗𝗔𝗔𝗟 𝗞𝗔𝗥 𝗖𝗛𝗨𝗗𝗔𝗜 𝗞𝗥𝗨𝗡𝗚𝗔",
    "𝗧𝗘𝗥𝗜 𝗕𝗘𝗛𝗡 𝗞𝗜 𝗖𝗛𝗨𝗧 𝗠𝗘𝗜𝗡 𝗔𝗚 𝗟𝗚𝗔 𝗞𝗔𝗥 𝟱𝟬𝟬 𝗟𝗜𝗧𝗔𝗥 𝗣𝗘𝗧𝗥𝗢𝗟 𝗗𝗔𝗔𝗟𝗨𝗡𝗚𝗔",
    "MADARCHOD", "BENCHOD", "RANDI KE PILLE",
    "TERI MAA KO CHOD DUNGA", "BHOSDIKE", "CHUTIYA"
};

// ==================== HTTP HELPER ====================
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::string urlEncode(const std::string& str) {
    std::string encoded;
    for (char c : str) {
        if (isalnum(c) || c == ' ' || c == '\n' || c == '.' || c == '_') {
            if (c == ' ') encoded += '+';
            else encoded += c;
        } else {
            char hex[4];
            snprintf(hex, sizeof(hex), "%%%02X", (unsigned char)c);
            encoded += hex;
        }
    }
    return encoded;
}

std::string httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

std::string httpPost(const std::string& url, const std::string& data) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

// ==================== TELEGRAM API ====================
std::string getBotToken() {
    // For userbot, we need to use MTProto, but for simplicity,
    // we'll use a demo mode that prints commands
    return "";
}

bool isAuthorized(int session_id, int64_t user_id) {
    if (user_id == MAIN_OWNER) return true;
    for (int64_t sudo : SUDO_USERS) {
        if (sudo == user_id) return true;
    }
    if (session_id < (int)SESSIONS.size() && SESSIONS[session_id].owner == user_id) return true;
    return false;
}

std::string getRandomAbuse() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, ABUSE_LIST.size() - 1);
    return ABUSE_LIST[dist(gen)];
}

std::string makeMention(int64_t user_id, const std::string& name = "") {
    std::string display = name.empty() ? std::to_string(user_id) : name;
    return "<a href=\"tg://user?id=" + std::to_string(user_id) + "\">" + display + "</a>";
}

std::string getTimestamp() {
    auto now = system_clock::now();
    auto time_t_now = system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%H:%M:%S");
    return ss.str();
}

void logMessage(const std::string& level, const std::string& msg) {
    std::cout << "[" << getTimestamp() << "] [" << level << "] " << msg << std::endl;
}

// ==================== SEND MESSAGE VIA BOT API ====================
void sendMessage(int session_id, int64_t chat_id, const std::string& text, 
                 int64_t reply_to = 0) {
    std::string encoded = urlEncode(text);
    
    // Since we don't have a bot token for userbot, we'll just log
    logMessage("SEND", "Chat " + std::to_string(chat_id) + ": " + text.substr(0, 100));
    
    // In production, you'd use MTProto here
    // For now, simulate sending
    std::this_thread::sleep_for(std::chrono::milliseconds(SEND_DELAY_MS));
}

// ==================== BOT STATE ====================
struct BotState {
    bool raid_active;
    double delay;
    int64_t target_user;
    int64_t target_chat;
    std::chrono::steady_clock::time_point last_msg;
    
    BotState() : raid_active(false), delay(0.3), target_user(0), target_chat(0) {}
};

std::map<int, BotState> states;
std::atomic<bool> running{true};

// ==================== COMMAND HANDLERS ====================
void handleRaid(int session_id, int64_t chat_id, int64_t target_id) {
    if (states[session_id].raid_active) {
        logMessage("WARN", "Raid already active");
        return;
    }
    
    states[session_id].raid_active = true;
    states[session_id].target_user = target_id;
    states[session_id].target_chat = chat_id;
    
    std::thread([session_id, chat_id, target_id]() {
        int count = 0;
        while (states[session_id].raid_active && running) {
            std::string abuse = getRandomAbuse();
            std::string mention = makeMention(target_id);
            std::string final_text = mention + " " + abuse;
            
            sendMessage(session_id, chat_id, final_text);
            count++;
            
            if (count % 10 == 0) {
                logMessage("INFO", "Raid sent " + std::to_string(count) + " messages");
            }
            
            double delay = states[session_id].delay;
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        }
        logMessage("INFO", "Raid stopped");
    }).detach();
    
    logMessage("INFO", "🔥 RAID started on " + std::to_string(target_id));
}

void stopRaid(int session_id) {
    states[session_id].raid_active = false;
    logMessage("INFO", "RAID stopped");
}

void setSpeed(int session_id, double delay) {
    if (delay < 0.1) delay = 0.1;
    if (delay > 5) delay = 5;
    states[session_id].delay = delay;
    logMessage("INFO", "Speed set to " + std::to_string(delay) + "s");
}

// ==================== LOAD CONFIG ====================
void loadConfig() {
    std::ifstream file("config.env");
    if (!file.is_open()) {
        logMessage("ERROR", "config.env not found!");
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        
        if (key == "API_ID") {
            API_ID = value;
        } else if (key == "API_HASH") {
            API_HASH = value;
        } else if (key == "MAIN_OWNER") {
            MAIN_OWNER = std::stoll(value);
        } else if (key == "SEND_DELAY_MS") {
            SEND_DELAY_MS = std::stoi(value);
        } else if (key == "SESSION_0") {
            Session s;
            s.index = 0;
            s.session_string = value;
            s.active = true;
            SESSIONS.push_back(s);
        } else if (key == "SESSION_0_OWNER") {
            if (!SESSIONS.empty()) {
                SESSIONS.back().owner = std::stoll(value);
            }
        }
    }
    file.close();
    
    logMessage("INFO", "Loaded " + std::to_string(SESSIONS.size()) + " sessions");
}

// ==================== COMMAND PROCESSOR ====================
void processCommand(int session_id, int64_t chat_id, int64_t user_id, const std::string& cmd) {
    if (!isAuthorized(session_id, user_id)) {
        logMessage("WARN", "Unauthorized command from " + std::to_string(user_id));
        return;
    }
    
    logMessage("CMD", "From " + std::to_string(user_id) + ": " + cmd);
    
    // .help
    if (cmd == ".help") {
        std::string help = 
            "🔥 VILLAIN USERBOT - C++ EDITION 🔥\n\n"
            "📌 COMMANDS:\n"
            "├ .ra @user - Start raid\n"
            "├ .stopra - Stop raid\n"
            "├ .dly 0.5 - Set speed (0.1-5s)\n"
            "├ .ping - Check bot\n"
            "├ .stats - Show stats\n"
            "└ .help - This menu\n\n"
            "⚡ Current speed: " + std::to_string(states[session_id].delay) + "s\n"
            "👑 Owner: " + std::to_string(MAIN_OWNER);
        sendMessage(session_id, chat_id, help);
        return;
    }
    
    // .ping
    if (cmd == ".ping") {
        auto start = steady_clock::now();
        sendMessage(session_id, chat_id, "🏓 Pong!");
        auto end = steady_clock::now();
        int ms = duration_cast<milliseconds>(end - start).count();
        logMessage("INFO", "Ping: " + std::to_string(ms) + "ms");
        return;
    }
    
    // .stats
    if (cmd == ".stats") {
        std::string stats = 
            "📊 STATS\n"
            "├ Raid: " + std::string(states[session_id].raid_active ? "Active" : "Stopped") + "\n"
            "├ Speed: " + std::to_string(states[session_id].delay) + "s\n"
            "└ Sessions: " + std::to_string(SESSIONS.size());
        sendMessage(session_id, chat_id, stats);
        return;
    }
    
    // .stopra
    if (cmd == ".stopra") {
        stopRaid(session_id);
        sendMessage(session_id, chat_id, "✅ RAID stopped");
        return;
    }
    
    // .ra @username or .ra user_id
    std::regex ra_regex(R"(\.ra\s+([@\d]+))");
    std::smatch match;
    if (std::regex_search(cmd, match, ra_regex) && match.size() > 1) {
        std::string target = match[1];
        int64_t target_id = 0;
        
        if (target[0] == '@') {
            // For demo, use a test ID
            target_id = 5390485406;
            logMessage("INFO", "Username mention: " + target);
        } else {
            target_id = std::stoll(target);
        }
        
        handleRaid(session_id, chat_id, target_id);
        sendMessage(session_id, chat_id, "🔥 RAID activated on " + makeMention(target_id));
        return;
    }
    
    // .dly X
    std::regex dly_regex(R"(\.dly\s+(\d+\.?\d*))");
    if (std::regex_search(cmd, match, dly_regex) && match.size() > 1) {
        double delay = std::stod(match[1]);
        setSpeed(session_id, delay);
        sendMessage(session_id, chat_id, "⚡ Speed set to " + std::to_string(delay) + "s");
        return;
    }
    
    // Unknown command
    if (cmd[0] == '.') {
        sendMessage(session_id, chat_id, "❌ Unknown command. Try .help");
    }
}

// ==================== MESSAGE QUEUE ====================
struct QueuedMessage {
    int session_id;
    int64_t chat_id;
    int64_t user_id;
    std::string text;
    bool is_command;
};

std::queue<QueuedMessage> msgQueue;
std::mutex queueMutex;
std::condition_variable queueCV;

void messageProcessor() {
    while (running) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCV.wait(lock, [] { return !msgQueue.empty() || !running; });
        
        if (!running) break;
        
        QueuedMessage msg = msgQueue.front();
        msgQueue.pop();
        lock.unlock();
        
        if (msg.is_command) {
            processCommand(msg.session_id, msg.chat_id, msg.user_id, msg.text);
        }
    }
}

// ==================== SIMULATED MESSAGE RECEIVER ====================
void simulateMessageReceiver(int session_id) {
    // In production, this would listen to actual Telegram updates
    // For demo, we'll read from stdin
    
    std::string input;
    while (running) {
        std::getline(std::cin, input);
        if (input.empty()) continue;
        
        if (input == "exit" || input == "quit") {
            running = false;
            break;
        }
        
        QueuedMessage msg;
        msg.session_id = session_id;
        msg.chat_id = -1001234567;  // Test chat
        msg.user_id = MAIN_OWNER;
        msg.text = input;
        msg.is_command = (input[0] == '.');
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            msgQueue.push(msg);
            queueCV.notify_one();
        }
    }
}

// ==================== SHOW BANNER ====================
void showBanner() {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "   🔥 VILLAIN USERBOT - C++ EDITION 🔥" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "   ⚡ Language: C++20" << std::endl;
    std::cout << "   🚀 Delay: " << SEND_DELAY_MS << "ms" << std::endl;
    std::cout << "   👑 Owner: " << MAIN_OWNER << std::endl;
    std::cout << "   📱 Sessions: " << SESSIONS.size() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "\n   🟢 Bot is running!" << std::endl;
    std::cout << "   📝 Type commands below:\n" << std::endl;
}

// ==================== MAIN ====================
int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    
    loadConfig();
    showBanner();
    
    if (SESSIONS.empty()) {
        std::cout << "❌ No sessions loaded! Add SESSION_0 in config.env" << std::endl;
        return 1;
    }
    
    // Initialize states
    for (const auto& session : SESSIONS) {
        states[session.index] = BotState();
        std::cout << "[✓] Session " << session.index << " initialized" << std::endl;
    }
    
    // Start message processor
    std::thread processor(messageProcessor);
    
    // Start command receiver
    std::thread receiver(simulateMessageReceiver, 0);
    
    // Show help
    std::cout << "\n📌 Available commands:" << std::endl;
    std::cout << "   .help     - Show all commands" << std::endl;
    std::cout << "   .ra 12345 - Start raid on user ID" << std::endl;
    std::cout << "   .stopra   - Stop raid" << std::endl;
    std::cout << "   .dly 0.5  - Set delay" << std::endl;
    std::cout << "   .ping     - Check bot" << std::endl;
    std::cout << "   .stats    - Show stats" << std::endl;
    std::cout << "   exit      - Stop bot" << std::endl;
    std::cout << "\n💡 Type commands and press Enter:\n" << std::endl;
    
    receiver.join();
    
    running = false;
    queueCV.notify_all();
    processor.join();
    
    curl_global_cleanup();
    
    std::cout << "\n👋 Shutdown complete!" << std::endl;
    return 0;
}
