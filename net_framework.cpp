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

#define GREEN   "\033[32m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

struct Config {
    std::string ip = "192.168.2.42";
    int start_port = 1;
    int end_port = 1000;
    int delay_ms = 2;
    int timeout_ms = 25;
};

void print_ascii_banner() {
    std::cout << GREEN << BOLD;
    std::cout << " ███▄    █ ▓█████ ▒██   ██▒ █    ██   ██████ \n";
    std::cout << " ██ ▀█   █ ▓█   ▀  ▒██  ██▒ ██  ▓██▒▒██    ▒ \n";
    std::cout << "▓██  ▀█ ██▒▒███     ▒██ ██░▓██  ▒██░░ ▓██▄   \n";
    std::cout << "▓██▒  ▐▌██▒▒▓█  ▄   ░ ▐██▓░▓▓  ░██░  ▒   ██▒\n";
    std::cout << "▒██░   ▓██░░▒████▒  ░ ██▒▓░▒▒████▓▒▒██████▒▒\n";
    std::cout << "░ ▒░   ▒ ▒ ░░ ▒░ ░   ██▒▒▒ ░▒▒▓  ▒ ▒ ▒▓▒ ▒ ░\n";
    std::cout << "                 [ NEXUS FRAMEWORK v4.0 ]    \n";
    std::cout << "-------------------------------------------------\n" << RESET;
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

void configure_settings(Config &cfg) {
    while (true) {
        std::cout << "\033[2J\033[H"; print_ascii_banner();
        std::cout << GREEN << BOLD << "[ OPTIONS / CONFIGURATION ]\n\n" << RESET;
        std::cout << GREEN << "1. Target IP aendern     (Aktuell: " << cfg.ip << ")\n";
        std::cout << "2. Port-Bereich aendern  (Aktuell: " << cfg.start_port << " - " << cfg.end_port << ")\n";
        std::cout << "3. Scan-Delay anpassen  (Aktuell: " << cfg.delay_ms << " ms)\n";
        std::cout << "4. Timeout anpassen     (Aktuell: " << cfg.timeout_ms << " ms)\n";
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

void start_clean_scan(const Config &cfg) {
    std::cout << "\n" << GREEN << BOLD << "[*] Running TCP Port-Scanner..." << RESET << "\n";
    int offene_ports = 0;
    int gesamt_ports = cfg.end_port - cfg.start_port + 1;

    for (int port = cfg.start_port; port <= cfg.end_port; ++port) {
        double prozent = (static_cast<double>(port - cfg.start_port + 1) / gesamt_ports) * 100.0;
        std::cout << "\r" << GREEN << "Progress: " << std::fixed;
        std::cout.precision(1);
        std::cout << prozent << "% [Scanning: " << port << "]" << RESET << std::flush;

        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock >= 0) {
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_NONBLOCK);

            sockaddr_in target;
            std::memset(&target, 0, sizeof(target));
            target.sin_family = AF_INET;
            target.sin_port = htons(port);
            inet_pton(AF_INET, cfg.ip.c_str(), &target.sin_addr);

            connect(sock, (struct sockaddr*)&target, sizeof(target));

            fd_set fdset; FD_ZERO(&fdset); FD_SET(sock, &fdset);
            timeval timeout = {0, cfg.timeout_ms * 1000};

            if (select(sock + 1, NULL, &fdset, NULL, &timeout) > 0) {
                int so_error; socklen_t len = sizeof(so_error);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0) {
                    std::cout  matrix_ports = {22, 80, 135, 139, 443, 445, 554, 1025, 3074, 5357, 8008, 8080, 9000, 5555, 62078}; 
    
    for (int i = 1; i <= 254; ++i) {
        std::string test_ip = "192.168.2." + std::to_string(i);
        std::cout << "\rProbing Host: " << test_ip << " ... " << std::flush;
        
        bool host_online = false;
        for (int port : matrix_ports) {
            int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock >= 0) {
                int flags = fcntl(sock, F_GETFL, 0);
                fcntl(sock, F_SETFL, flags | O_NONBLOCK);
                
                sockaddr_in target;
                std::memset(&target, 0, sizeof(target));
                target.sin_family = AF_INET;
                target.sin_port = htons(port);
                inet_pton(AF_INET, test_ip.c_str(), &target.sin_addr);
                
                connect(sock, (struct sockaddr*)&target, sizeof(target));
                fd_set fdset; FD_ZERO(&fdset); FD_SET(sock, &fdset);
                timeval timeout = {0, 10000};
                
                if (select(sock + 1, NULL, &fdset, NULL, &timeout) > 0) {
                    host_online = true;
                    close(sock);
                    break; 
                }
                close(sock);
            }
        }
        if (host_online) {
            std::cout  auswahl)) { std::cin.clear(); std::cin.ignore(10000, '\n'); continue; }

        if (auswahl == 1) start_clean_scan(cfg);
        else if (auswahl == 2) resolve_device_identity(cfg);
        else if (auswahl == 3) scan_local_network_hosts();
        else if (auswahl == 4) configure_settings(cfg);
        else if (auswahl == 5) { std::cout << GREEN << "\nExiting. Stay safe.\n" << RESET; break; }
    }
    return 0;
}
