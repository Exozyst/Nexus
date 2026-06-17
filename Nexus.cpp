cd ~ && rm -rf nexus* && cat << 'EOF' > nexus.cpp
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <memory>
#include <chrono>
#include <thread>

#define GREEN   "\033[32m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

struct Config {
    std::string ip = "192.168.2.42";
    int start_port = 1;
    int end_port = 1000;
    int delay_ms = 2;
    int timeout_ms = 50; // Etwas erhoeht fuer mehr Stabilitaet
};

// Nutzt Raw-String-Literals (R"()"), um Escaping-Fehler beim ASCII-Art zu vermeiden
void print_ascii_banner() {
    std::cout << GREEN << BOLD;
    std::cout << R"(
 ███▄    █ ▓█████ ▒██   ██▒ █    ██   ██████ 
 ██ ▀█   █ ▓█   ▀  ▒██  ██▒ ██  ▓██▒▒██    ▒ 
▓██  ▀█ ██▒▒███     ▒██ ██░▓██  ▒██░░ ▓██▄   
▓██▒  ▐▌██▒▒▓█  ▄   ░ ▐██▓░▓▓  ░██░  ▒   ██▒
▒██░   ▓██░░▒████▒  ░ ██▒▓░▒▒████▓▒▒██████▒▒
░ ▒░   ▒ ▒ ░░ ▒░ ░   ██▒▒▒ ░▒▒▓  ▒ ▒ ▒▓▒ ▒ ░
                 [ NEXUS FRAMEWORK v4.0 ]    
-------------------------------------------------
)" << RESET;
}

void show_loading_bar() {
    std::cout << GREEN << "\n[*] Initializing Nexus Modules...\n" << RESET;
    const int barWidth = 40;
    for (int i = 0; i <= barWidth; ++i) {
        std::cout << GREEN << BOLD << "\r[";
        int pos = i;
        for (int j = 0; j < barWidth; ++j) {
            if (j < pos) std::cout << "=";
            else if (j == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int((i / (float)barWidth) * 100.0) << "%" << RESET << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    std::cout << "\n\n";
}

std::string execute_system_command(const std::string& cmd) {
    char buffer[128];
    std::string result = "";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "Fehler: Engine konnte nicht aufgerufen werden.";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

void start_clean_scan(const Config &cfg) {
    std::cout << "\n" << GREEN << BOLD << "[*] Running TCP Port-Scanner on " << cfg.ip << "..." << RESET << "\n";
    int offene_ports = 0;
    int gesamt_ports = cfg.end_port - cfg.start_port + 1;

    for (int port = cfg.start_port; port <= cfg.end_port; ++port) {
        double prozent = (static_cast<double>(port - cfg.start_port + 1) / gesamt_ports) * 100.0;
        std::cout << "\r" << GREEN << "Progress: " << std::fixed;
        std::cout.precision(1);
        std::cout << prozent << "% [Scanning: " << port << "]" << RESET << std::flush;

        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) continue;

        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        sockaddr_in target;
        std::memset(&target, 0, sizeof(target));
        target.sin_family = AF_INET;
        target.sin_port = htons(port);
        inet_pton(AF_INET, cfg.ip.c_str(), &target.sin_addr);

        connect(sock, (struct sockaddr*)&target, sizeof(target));

        fd_set fdset; 
        FD_ZERO(&fdset); 
        FD_SET(sock, &fdset);
        timeval timeout = {0, cfg.timeout_ms * 1000};

        if (select(sock + 1, NULL, &fdset, NULL, &timeout) > 0) {
            int so_error; 
            socklen_t len = sizeof(so_error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
            if (so_error == 0) {
                std::cout << "\n[+] Port " << port << " is OPEN" << std::flush;
                offene_ports++;
            }
        }
        close(sock);
        std::this_thread::sleep_for(std::chrono::milliseconds(cfg.delay_ms));
    }
    std::cout << "\n" << GREEN << "[*] Scan complete. Found " << offene_ports << " open ports.\n" << RESET;
}

void scan_local_network_hosts() {
    std::cout << "\n" << GREEN << BOLD << "[*] Advanced Subnet Scan (via Nmap Chaining)..." << RESET << "\n";
    std::cout << "Pruefe auf nmap Installation...\n";
    
    std::string check_nmap = execute_system_command("command -v nmap");
    if (check_nmap.empty()) {
        std::cout << "[-] Nmap ist nicht installiert! Bitte 'pkg install nmap' in Termux ausfuehren.\n";
        std::cout << "[-] Falle zurueck auf langsamen TCP-Connect-Scan...\n";
        // Hier koennte dein alter C++ Code als Fallback stehen
        return;
    }

    std::cout << "[+] Nmap gefunden! Starte schnellen ARP/Ping Sweep auf 192.168.2.0/24...\n\n";
    
    // Der -sn Parameter macht einen schnellen Ping-Sweep. 
    // -T4 erhoeht die Geschwindigkeit (Timing Template).
    std::string nmap_cmd = "nmap -sn -T4 192.168.2.0/24 | grep -E 'Nmap scan report|MAC Address'";
    std::string result = execute_system_command(nmap_cmd);
    
    if (result.empty()) {
        std::cout << "[-] Scan fehlgeschlagen oder keine Geraete gefunden. (Root-Rechte fuer MAC-Adressen fehlen eventuell)\n";
    } else {
        std::cout << result << "\n";
    }
    
    std::cout << GREEN << "[*] Subnet Scan complete.\n" << RESET;
}

void resolve_device_identity(const Config &cfg) {
    std::cout << "\n" << GREEN << BOLD << "[*] Resolving Device Identity for " << cfg.ip << "..." << RESET << "\n";
    // Nutzt ARP als Fallback fuer lokale Identifikation in Termux
    std::string cmd = "arp -a | grep " + cfg.ip;
    std::string output = execute_system_command(cmd);
    if (output.empty()) {
        std::cout << "[-] Konnte keine ARP-Daten fuer diese IP finden. Versuche nmap...\n";
        std::string nmap_cmd = "nmap -O -sV " + cfg.ip + " 2>/dev/null";
        std::cout << execute_system_command(nmap_cmd) << "\n";
    } else {
        std::cout << "[+] ARP Table Entry:\n" << output << "\n";
    }
}

void read_system_specs() {
    std::cout << "\n" << GREEN << BOLD << "[*] Fetching Client System Specifications..." << RESET << "\n";
    
    // Android Device Model via getprop
    std::cout << BOLD << "Device Model: " << RESET;
    std::cout << execute_system_command("getprop ro.product.brand | tr -d '\n' && echo -n ' ' && getprop ro.product.model");
    
    // Android OS Version
    std::cout << BOLD << "Android Version: " << RESET;
    std::cout << execute_system_command("getprop ro.build.version.release");
    
    // CPU Info (Architektur)
    std::cout << BOLD << "CPU Arch: " << RESET;
    std::cout << execute_system_command("uname -m");
    
    // RAM Info (aus /proc/meminfo)
    std::cout << BOLD << "Memory: " << RESET;
    std::cout << execute_system_command("grep MemTotal /proc/meminfo | awk '{print $2/1024 \" MB\"}'");
    
    std::cout << GREEN << "[*] System-Check abgeschlossen.\n" << RESET;
}

void configure_settings(Config &cfg) {
    while (true) {
        std::cout << "\033[2J\033[H"; print_ascii_banner();
        std::cout << GREEN << BOLD << "[ OPTIONS / CONFIGURATION ]\n\n" << RESET;
        std::cout << GREEN << "1. Target IP aendern     (Aktuell: " << cfg.ip << ")\n";
        std::cout << "2. Port-Bereich aendern  (Aktuell: " << cfg.start_port << " - " << cfg.end_port << ")\n";
        std::cout << "3. Scan-Delay anpassen   (Aktuell: " << cfg.delay_ms << " ms)\n";
        std::cout << "4. Timeout anpassen      (Aktuell: " << cfg.timeout_ms << " ms)\n";
        std::cout << "5. ZURUECK ZUM HAUPTMENUE\n" << RESET;
        std::cout << GREEN << "\nroot@nexus:~# ";
        int wahl;
        if (!(std::cin >> wahl)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }
        if (wahl == 1) { std::cout << "Neue IP: "; std::cin >> cfg.ip; }
        else if (wahl == 2) { std::cout << "Start-Port: "; std::cin >> cfg.start_port; std::cout << "End-Port: "; std::cin >> cfg.end_port; }
        else if (wahl == 3) { std::cout << "Delay (ms): "; std::cin >> cfg.delay_ms; }
        else if (wahl == 4) { std::cout << "Timeout (ms): "; std::cin >> cfg.timeout_ms; }
        else if (wahl == 5) break;
    }
}

int main() {
    std::cout << "\033[2J\033[H";
    Config cfg;
    show_loading_bar();
    
    while (true) {
        print_ascii_banner();
        std::cout << "1. Run TCP Port-Test (Target: " << cfg.ip << ")\n";
        std::cout << "2. Resolve Device Identity (ARP/nmap wrap)\n";
        std::cout << "3. Map Active WLAN-Hosts (Advanced Subnet Scan)\n";
        std::cout << "4. Configure Parameters (Settings)\n";
        std::cout << "5. Client System Specs (NEW)\n";
        std::cout << "6. Exit Framework\n";
        std::cout << "\nroot@nexus:~# ";
        
        int auswahl;
        if (!(std::cin >> auswahl)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }

        if (auswahl == 1) start_clean_scan(cfg);
        else if (auswahl == 2) resolve_device_identity(cfg);
        else if (auswahl == 3) scan_local_network_hosts();
        else if (auswahl == 4) configure_settings(cfg);
        else if (auswahl == 5) read_system_specs();
        else if (auswahl == 6) { std::cout << GREEN << "\nExiting. Stay safe.\n" << RESET; break; }
        
        std::cout << "\nDruecke Enter um fortzufahren...";
        std::cin.ignore(10000, '\n');
        std::cin.get();
        std::cout << "\033[2J\033[H";
    }
    return 0;
}
EOF
clang++ -O3 nexus.cpp -o nexus && ./nexus
