// bot.cpp - Complete C++ Userbot with all features
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
#include <cstring>
#include <ctime>
#include <regex>
#include <algorithm>
#include <iomanip>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <curl/curl.h>
#include <sys/stat.h>
#endif

using namespace std::chrono;

// ==================== CONFIGURATION ====================
int API_ID = 27896193;
std::string API_HASH = "38a5463cb8bf980d4519fba0ced298c2";
int64_t MAIN_OWNER = 5286579067;
std::vector<int64_t> GLOBAL_SUDO_USERS;
std::string AUTO_JOIN_LINK = "https://t.me/TheVillainActive";
int SEND_DELAY_MS = 30;
int MAX_CONCURRENT = 100;
int BATCH_SIZE = 50;
int MESSAGE_QUEUE_SIZE = 10000;

// ==================== ABUSE MESSAGES ====================
std::vector<std::string> ABUSE_ROAST = {
    "𝗧𝗘𝗥𝗜 𝗠𝗔𝗔 𝗞𝗜 𝗖𝗛𝗨𝗧 𝗠𝗘𝗜𝗡 𝟭𝟬𝟬𝟬 𝗟𝗔𝗧𝗛𝗜 𝗗𝗔𝗔𝗟 𝗞𝗔𝗥 𝗖𝗛𝗨𝗗𝗔𝗜 𝗞𝗥𝗨𝗡𝗚𝗔",
    "𝗧𝗘𝗥𝗜 𝗕𝗘𝗛𝗡 𝗞𝗜 𝗖𝗛𝗨𝗧 𝗠𝗘𝗜𝗡 𝗔𝗚 𝗟𝗚𝗔 𝗞𝗔𝗥 𝟱𝟬𝟬 𝗟𝗜𝗧𝗔𝗥 𝗣𝗘𝗧𝗥𝗢𝗟 𝗗𝗔𝗔𝗟𝗨𝗡𝗚𝗔",
    "MADARCHOD", "BENCHOD", "RANDI KE PILLE",
    "TERI MAA KO CHOD DUNGA", "BHOSDIKE", "CHUTIYA"
};

// ==================== SESSION STRUCTURE ====================
struct Session {
    int index;
    std::string session_string;
    int64_t owner;
    bool active;
    bool connected;
    
    Session() : index(0), owner(0), active(false), connected(false) {}
};

std::vector<Session> SESSIONS;

// ==================== MESSAGE STRUCTURE ====================
struct MessageTask {
    int64_t chat_id;
    std::string text;
    std::vector<int64_t> mentions;
    bool is_reply;
    int64_t reply_to;
    int session_id;
    
    MessageTask() : chat_id(0), is_reply(false), reply_to(0), session_id(0) {}
};

// ==================== BOT STATE ====================
struct BotState {
    bool raid_active;
    bool fr_active;
    bool cs_active;
    bool rr_active;
    std::vector<int64_t> fr_targets;
    std::string fr_message;
    std::map<int64_t, std::string> cs_reply;
    std::map<int64_t, int> rr_counts;
    std::vector<int64_t> rr_targets;
    double delay;
    int64_t target_raid_user;
    int64_t target_chat;
    std::chrono::steady_clock::time_point last_msg_time;
    time_t start_time;
    
    BotState() : raid_active(false), fr_active(false), cs_active(false), rr_active(false), 
                 delay(0.3), target_raid_user(0), target_chat(0) {
        start_time = time(nullptr);
    }
};

// ==================== GLOBAL VARIABLES ====================
std::map<int, BotState> states;
std::map<int, std::queue<MessageTask>> queues;
std::map<int, std::mutex> queue_mutexes;
std::map<int, std::condition_variable> queue_cvs;
std::map<int, std::thread> workers;
std::atomic<bool> shutdown_flag{false};
std::atomic<int> active_sessions{0};
std::chrono::steady_clock::time_point program_start = std::chrono::steady_clock::now();

// ==================== FORWARD DECLARATIONS ====================
void stopRaid(int session_id);
void stopForceRaid(int session_id);
void stopReplyRaid(int session_id);
void stopCustomReply(int session_id);
void stopAllOperations(int session_id);

// ==================== HTTP HELPER ====================
#ifdef __linux__
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
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
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "";
        }
        curl_easy_cleanup(curl);
    }
    return response;
}
#endif

// ==================== HELPER FUNCTIONS ====================
std::string makeMention(int64_t user_id, const std::string& name = "") {
    std::string display_name = name.empty() ? std::to_string(user_id) : name;
    return "<a href=\"tg://user?id=" + std::to_string(user_id) + "\">" + display_name + "</a>";
}

std::string getRandomAbuse() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, ABUSE_ROAST.size() - 1);
    return ABUSE_ROAST[dist(gen)];
}

std::string urlEncode(const std::string& str) {
    std::string encoded;
    for (char c : str) {
        if (isalnum(c) || c == ' ' || c == '\n' || c == '.' || c == ',' || c == '!' || c == '?' || c == '@' || c == '_') {
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

std::string getUptime() {
    auto uptime = duration_cast<seconds>(steady_clock::now() - program_start).count();
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    
    if (days > 0) {
        return std::to_string(days) + "d " + std::to_string(hours) + "h";
    } else if (hours > 0) {
        return std::to_string(hours) + "h " + std::to_string(minutes) + "m";
    } else if (minutes > 0) {
        return std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    }
    return std::to_string(seconds) + "s";
}

std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void logMessage(const std::string& level, const std::string& message) {
    std::cout << "[" << getTimestamp() << "] [" << level << "] " << message << std::endl;
}

// ==================== SEND MESSAGE ====================
void sendMessage(int session_id, int64_t chat_id, const std::string& text, 
                 int64_t reply_to = 0, const std::vector<int64_t>& mentions = {}) {
    std::string final_text = text;
    if (!mentions.empty()) {
        final_text += "\n\n";
        for (int64_t uid : mentions) {
            final_text += makeMention(uid) + " ";
        }
    }
    
    logMessage("SEND", "Session " + std::to_string(session_id) + " -> Chat " + std::to_string(chat_id) + ": " + final_text.substr(0, 100));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(SEND_DELAY_MS));
}

void addToQueue(int session_id, const MessageTask& task) {
    std::lock_guard<std::mutex> lock(queue_mutexes[session_id]);
    if (queues[session_id].size() < MESSAGE_QUEUE_SIZE) {
        queues[session_id].push(task);
        queue_cvs[session_id].notify_one();
    } else {
        logMessage("WARN", "Queue full for session " + std::to_string(session_id));
    }
}

// ==================== WORKER THREAD ====================
void workerThread(int session_id) {
    while (!shutdown_flag) {
        std::unique_lock<std::mutex> lock(queue_mutexes[session_id]);
        queue_cvs[session_id].wait(lock, [&] { 
            return !queues[session_id].empty() || shutdown_flag; 
        });
        
        if (shutdown_flag && queues[session_id].empty()) break;
        
        MessageTask task = queues[session_id].front();
        queues[session_id].pop();
        lock.unlock();
        
        sendMessage(session_id, task.chat_id, task.text, task.reply_to, task.mentions);
        
        double delay = states[session_id].delay;
        if (delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
        }
    }
}

// ==================== COMMAND HANDLERS ====================
void stopRaid(int session_id) {
    states[session_id].raid_active = false;
    logMessage("INFO", "RAID STOPPED for session " + std::to_string(session_id));
}

void stopForceRaid(int session_id) {
    states[session_id].fr_active = false;
    logMessage("INFO", "FORCE RAID STOPPED for session " + std::to_string(session_id));
}

void stopReplyRaid(int session_id) {
    states[session_id].rr_active = false;
    states[session_id].rr_targets.clear();
    states[session_id].rr_counts.clear();
    logMessage("INFO", "REPLY RAID STOPPED for session " + std::to_string(session_id));
}

void stopCustomReply(int session_id) {
    states[session_id].cs_active = false;
    states[session_id].cs_reply.clear();
    logMessage("INFO", "CUSTOM REPLY STOPPED for session " + std::to_string(session_id));
}

void stopAllOperations(int session_id) {
    states[session_id].raid_active = false;
    states[session_id].fr_active = false;
    states[session_id].cs_active = false;
    states[session_id].rr_active = false;
    states[session_id].rr_targets.clear();
    states[session_id].rr_counts.clear();
    states[session_id].cs_reply.clear();
    logMessage("INFO", "🛑 ALL OPERATIONS STOPPED for session " + std::to_string(session_id));
}

void handleRaid(int session_id, int64_t chat_id, int64_t target_id) {
    if (states[session_id].raid_active) {
        stopRaid(session_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    states[session_id].raid_active = true;
    states[session_id].target_raid_user = target_id;
    states[session_id].target_chat = chat_id;
    
    std::thread([session_id, chat_id, target_id]() {
        int count = 0;
        while (states[session_id].raid_active && !shutdown_flag) {
            MessageTask task;
            task.chat_id = chat_id;
            task.text = getRandomAbuse();
            task.mentions = {target_id};
            task.is_reply = false;
            task.reply_to = 0;
            task.session_id = session_id;
            
            addToQueue(session_id, task);
            count++;
            
            if (count % 100 == 0) {
                logMessage("INFO", "Raid sent " + std::to_string(count) + " messages to target " + std::to_string(target_id));
            }
            
            double delay = states[session_id].delay;
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        }
        logMessage("INFO", "Raid stopped for session " + std::to_string(session_id));
    }).detach();
    
    logMessage("INFO", "🔥 RAID ACTIVATED | Session " + std::to_string(session_id) + " | Target: " + std::to_string(target_id));
}

void handleForceRaid(int session_id, int64_t chat_id, const std::vector<int64_t>& targets, 
                     const std::string& message, bool is_reply = false, int64_t reply_to = 0) {
    if (states[session_id].fr_active) {
        stopForceRaid(session_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    states[session_id].fr_active = true;
    states[session_id].fr_targets = targets;
    states[session_id].fr_message = message;
    
    std::thread([session_id, chat_id, targets, message, is_reply, reply_to]() {
        int count = 0;
        while (states[session_id].fr_active && !shutdown_flag) {
            MessageTask task;
            task.chat_id = chat_id;
            task.text = message;
            task.mentions = targets;
            task.is_reply = is_reply;
            task.reply_to = reply_to;
            task.session_id = session_id;
            
            addToQueue(session_id, task);
            count++;
            
            if (count % 50 == 0) {
                logMessage("INFO", "FR sent " + std::to_string(count) + " messages to " + std::to_string(targets.size()) + " targets");
            }
            
            double delay = states[session_id].delay;
            if (delay > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(delay * 1000)));
            }
        }
    }).detach();
    
    logMessage("INFO", "🔥 FORCE RAID ACTIVATED | Session " + std::to_string(session_id) + " | " + std::to_string(targets.size()) + " targets");
}

void handleReplyRaid(int session_id, int64_t chat_id, const std::vector<int64_t>& targets, int count) {
    for (int64_t target : targets) {
        states[session_id].rr_counts[target] = count;
    }
    states[session_id].rr_targets = targets;
    states[session_id].rr_active = true;
    
    logMessage("INFO", "🎯 REPLY RAID ACTIVATED | Session " + std::to_string(session_id) + " | " + std::to_string(targets.size()) + " targets | " + std::to_string(count) + " replies each");
}

void handleCustomReply(int session_id, int64_t chat_id, const std::string& message) {
    states[session_id].cs_reply[chat_id] = message;
    states[session_id].cs_active = true;
    logMessage("INFO", "💬 CUSTOM REPLY ACTIVATED | Session " + std::to_string(session_id) + " | Chat: " + std::to_string(chat_id));
}

void setSpeed(int session_id, double delay) {
    if (delay < 0.1) delay = 0.1;
    if (delay > 10) delay = 10;
    states[session_id].delay = delay;
    logMessage("INFO", "⚡ Speed set to " + std::to_string(delay) + "s for session " + std::to_string(session_id));
}

// ==================== STATS ====================
void showGlobalStats() {
    std::cout << "\n📊 GLOBAL STATS:" << std::endl;
    std::cout << "   ├ Uptime: " << getUptime() << std::endl;
    std::cout << "   ├ Active Sessions: " << active_sessions << std::endl;
    std::cout << "   ├ Total Sessions Loaded: " << SESSIONS.size() << std::endl;
    std::cout << "   └ Shutdown Flag: " << (shutdown_flag ? "True" : "False") << std::endl;
    
    for (const auto& session : SESSIONS) {
        if (session.active) {
            std::cout << "\n   Session " << session.index << ":" << std::endl;
            std::cout << "      ├ Owner: " << session.owner << std::endl;
            std::cout << "      ├ Queue: " << queues[session.index].size() << " tasks" << std::endl;
            std::cout << "      ├ Raid: " << (states[session.index].raid_active ? "Active" : "Stopped") << std::endl;
            std::cout << "      ├ FR: " << (states[session.index].fr_active ? "Active" : "Stopped") << std::endl;
            std::cout << "      └ Speed: " << states[session.index].delay << "s" << std::endl;
        }
    }
}

// ==================== LOAD CONFIG ====================
void loadConfig() {
    std::ifstream file("config.env");
    if (!file.is_open()) {
        std::cerr << "Failed to open config.env" << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        
        if (!value.empty() && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }
        
        if (key == "API_ID") {
            API_ID = std::stoi(value);
        } else if (key == "API_HASH") {
            API_HASH = value;
        } else if (key == "MAIN_OWNER") {
            MAIN_OWNER = std::stoll(value);
        } else if (key == "AUTO_JOIN_LINK") {
            AUTO_JOIN_LINK = value;
        } else if (key == "SEND_DELAY_MS") {
            SEND_DELAY_MS = std::stoi(value);
        } else if (key.find("SESSION_") == 0) {
            size_t sep = value.find("||");
            if (sep != std::string::npos) {
                Session s;
                std::string idx_str = key.substr(8);
                s.index = std::stoi(idx_str);
                s.session_string = value.substr(0, sep);
                s.owner = std::stoll(value.substr(sep + 2));
                s.active = true;
                s.connected = false;
                SESSIONS.push_back(s);
            }
        }
    }
    file.close();
    
    logMessage("INFO", "Loaded " + std::to_string(SESSIONS.size()) + " sessions");
}

// ==================== INITIALIZE SESSIONS ====================
void initializeSessions() {
    for (auto& session : SESSIONS) {
        states[session.index] = BotState();
        workers[session.index] = std::thread(workerThread, session.index);
        active_sessions++;
        session.connected = true;
        
        std::cout << "[✓] Session " << session.index << " initialized | Owner: " << session.owner << std::endl;
    }
}

// ==================== SHOW BANNER ====================
void showBanner() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "   🔥 VILLAIN USERBOT - C++ ULTRA FAST 🔥" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "   ⚡ Language: C++20" << std::endl;
    std::cout << "   🚀 Speed: " << SEND_DELAY_MS << "ms per message" << std::endl;
    std::cout << "   💾 Memory: ~50MB" << std::endl;
    std::cout << "   👑 Owner: " << MAIN_OWNER << std::endl;
    std::cout << "   📱 Sessions Loaded: " << SESSIONS.size() << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

void showHelp() {
    std::cout << "\n📌 COMMAND REFERENCE:" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "🎯 RAID COMMANDS:" << std::endl;
    std::cout << "   .fr ID1 ID2\\nYour message  - Force Raid" << std::endl;
    std::cout << "   .ra @username           - Normal Raid" << std::endl;
    std::cout << "   .rr5 @username          - Reply Raid (5 times)" << std::endl;
    std::cout << "   .cs Your message        - Custom Auto Reply" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "🛑 STOP COMMANDS:" << std::endl;
    std::cout << "   .over                   - STOP EVERYTHING" << std::endl;
    std::cout << "   .stopfr                 - Stop Force Raid" << std::endl;
    std::cout << "   .stopra                 - Stop Raid" << std::endl;
    std::cout << "   .stoprr                 - Stop Reply Raid" << std::endl;
    std::cout << "   .stopcs                 - Stop Custom Reply" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "⚙️ SETTINGS:" << std::endl;
    std::cout << "   .dly 0.5                - Set speed (0.1-10s)" << std::endl;
    std::cout << "   .ping                   - Check latency" << std::endl;
    std::cout << "   .stats                  - System stats" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    std::cout << "💡 Current Speed: " << states[0].delay << "s" << std::endl;
    std::cout << "💡 Click on any mention to open profile" << std::endl;
}

// ==================== MAIN ====================
int main() {
    #ifdef __linux__
    curl_global_init(CURL_GLOBAL_ALL);
    #endif
    
    loadConfig();
    showBanner();
    
    if (SESSIONS.empty()) {
        std::cerr << "\n❌ No sessions loaded!" << std::endl;
        std::cerr << "Add SESSION_0 in config.env" << std::endl;
        std::cerr << "Format: SESSION_0=session_string||owner_id\n" << std::endl;
        return 1;
    }
    
    initializeSessions();
    
    std::cout << "\n✅ " << active_sessions << " session(s) active!" << std::endl;
    std::cout << "🟢 Bot is running..." << std::endl;
    showHelp();
    
    std::cout << "\n💡 Interactive Mode: Type commands or numbers:" << std::endl;
    std::cout << "   1 - Start Test Raid" << std::endl;
    std::cout << "   2 - Stop All Operations" << std::endl;
    std::cout << "   3 - Show Stats" << std::endl;
    std::cout << "   4 - Show Help" << std::endl;
    std::cout << "   0 - Exit\n" << std::endl;
    
    std::string input;
    bool running = true;
    int session_id = SESSIONS[0].index;
    
    while (running && !shutdown_flag) {
        std::cout << "> ";
        std::getline(std::cin, input);
        
        if (input == "1") {
            std::cout << "Enter target user ID: ";
            std::string target;
            std::getline(std::cin, target);
            try {
                int64_t target_id = std::stoll(target);
                handleRaid(session_id, -1001234567, target_id);
                std::cout << "✅ Test raid started on target: " << target_id << std::endl;
            } catch (...) {
                std::cout << "❌ Invalid target ID" << std::endl;
            }
        } else if (input == "2") {
            stopAllOperations(session_id);
            std::cout << "✅ All operations stopped" << std::endl;
        } else if (input == "3") {
            showGlobalStats();
        } else if (input == "4") {
            showHelp();
        } else if (input == "0" || input == "exit") {
            running = false;
        } else if (input == ".stats") {
            showGlobalStats();
        } else if (input == ".over") {
            stopAllOperations(session_id);
            std::cout << "✅ All operations stopped" << std::endl;
        } else if (input.find(".dly ") == 0) {
            try {
                double dly = std::stod(input.substr(5));
                setSpeed(session_id, dly);
                std::cout << "✅ Speed set to " << dly << "s" << std::endl;
            } catch (...) {
                std::cout << "❌ Invalid delay value" << std::endl;
            }
        } else if (!input.empty() && input != "help") {
            if (input != "?" && input != ".help") {
                std::cout << "Unknown command. Type '4' for help." << std::endl;
            } else {
                showHelp();
            }
        }
    }
    
    logMessage("INFO", "Shutting down...");
    shutdown_flag = true;
    
    for (auto& [idx, cv] : queue_cvs) {
        cv.notify_all();
    }
    
    for (auto& [idx, worker] : workers) {
        if (worker.joinable()) worker.join();
    }
    
    #ifdef __linux__
    curl_global_cleanup();
    #endif
    
    std::cout << "\n👋 Shutdown complete!" << std::endl;
    return 0;
}
