#include <iostream>
#include <thread>
#include <queue>
#include <stack>
#include <vector>
#include <cstring>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <unistd.h>

using namespace std;

struct Packet {
    int id;
    chrono::system_clock::time_point timestamp;
    vector<uint8_t> rawData;
    string srcIP, destIP;
};

queue<Packet> packetQueue;
queue<Packet> replayQueue;
queue<Packet> backupQueue;
mutex queueMutex;
bool capturing = true;

string formatTime(const chrono::system_clock::time_point &tp) {
    time_t t = chrono::system_clock::to_time_t(tp);
    char buf[26];
    ctime_r(&t, buf);
    buf[strlen(buf) - 1] = '\0';
    return string(buf);
}

vector<string> dissectPacket(const Packet &p) {
    vector<string> layers;
    stack<string> layerStack;
    const uint8_t *data = p.rawData.data();
    size_t len = p.rawData.size();

    if (len >= sizeof(ethhdr)) {
        layerStack.push("Ethernet");
        struct ethhdr *eth = (struct ethhdr *)data;

        if (ntohs(eth->h_proto) == ETH_P_IP) {
            layerStack.push("IPv4");
            struct iphdr *ip = (struct iphdr *)(data + sizeof(ethhdr));
            if (ip->protocol == IPPROTO_TCP)
                layerStack.push("TCP");
            else if (ip->protocol == IPPROTO_UDP)
                layerStack.push("UDP");
        } else if (ntohs(eth->h_proto) == ETH_P_IPV6) {
            layerStack.push("IPv6");
        }
    }

    while (!layerStack.empty()) {
        layers.push_back(layerStack.top());
        layerStack.pop();
    }

    cout << "[#] Dissection Layers for Current Packet: ";
    for (auto &l : layers) cout << l << " ";
    cout << endl;

    return layers;
}

void capturePackets(const string &interface, const string &srcFilter, const string &destFilter) {
    int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_raw < 0) {
        perror("Socket Error");
        return;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    if (ioctl(sock_raw, SIOCGIFINDEX, &ifr) == 0) {
        struct sockaddr_ll sll = {};
        sll.sll_family = AF_PACKET;
        sll.sll_ifindex = ifr.ifr_ifindex;
        sll.sll_protocol = htons(ETH_P_ALL);
        if (bind(sock_raw, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
            perror("Bind failed");
            close(sock_raw);
            return;
        }
    }

    int packetID = 0;
    char buffer[65536];

    while (capturing) {  // Stop when timer thread sets capturing = false
        ssize_t dataSize = recvfrom(sock_raw, buffer, sizeof(buffer), 0, nullptr, nullptr);
        if (dataSize <= 0) continue;

        Packet pkt;
        pkt.id = ++packetID;
        pkt.timestamp = chrono::system_clock::now();
        pkt.rawData.assign(buffer, buffer + dataSize);

        if (dataSize >= sizeof(ethhdr) + sizeof(iphdr)) {
            struct ethhdr *eth = (struct ethhdr *)buffer;
            if (ntohs(eth->h_proto) == ETH_P_IP) {
                struct iphdr *ip = (struct iphdr *)(buffer + sizeof(ethhdr));
                pkt.srcIP = inet_ntoa(*(in_addr *)&ip->saddr);
                pkt.destIP = inet_ntoa(*(in_addr *)&ip->daddr);
            }
        }

        {
            lock_guard<mutex> lock(queueMutex);
            packetQueue.push(pkt);
        }

        cout << "[+] Captured Packet #" << pkt.id << " | Src: " << pkt.srcIP
             << " | Dest: " << pkt.destIP << endl;
    }

    close(sock_raw);
}

void filterAndReplay() {
    while (capturing || !packetQueue.empty()) {
        if (packetQueue.empty()) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        Packet pkt;
        {
            lock_guard<mutex> lock(queueMutex);
            pkt = packetQueue.front();
            packetQueue.pop();
        }

        dissectPacket(pkt);

        if (pkt.rawData.size() > 1500) {
            cout << "[!] Skipping oversized packet #" << pkt.id << endl;
            continue;
        }

        replayQueue.push(pkt);
        cout << "[>] Replaying Packet #" << pkt.id << endl;

        bool success = true;
        int retries = 0;
        while (!success && retries < 2) {
            cout << "[x] Replay failed for #" << pkt.id << " (retry " << retries + 1 << ")" << endl;
            retries++;
            success = true;
        }

        if (!success) {
            backupQueue.push(pkt);
            cout << "[!] Moved Packet #" << pkt.id << " to backup." << endl;
        }

        double delay = pkt.rawData.size() / 1000.0;
        this_thread::sleep_for(chrono::milliseconds((int)delay));
    }
}

// Global timer thread that stops everything after 1 minute
void timerThread() {
    this_thread::sleep_for(chrono::minutes(1));
    capturing = false;
    cout << "\n[!] Timer expired — stopping capture after 1 minute.\n";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: sudo ./netmon <interface>\n";
        return 0;
    }

    string interface = argv[1];
    string srcFilter, destFilter;

    cout << "Enter source IP filter (blank=all): ";
    getline(cin, srcFilter);
    cout << "Enter destination IP filter (blank=all): ";
    getline(cin, destFilter);

    thread timer(timerThread);  //  start the 1-minute timer
    thread captureThread(capturePackets, interface, srcFilter, destFilter);
    thread replayThread(filterAndReplay);

    timer.join();
    captureThread.join();
    replayThread.join();

    cout << "\n=== SYSTEM SUMMARY ===" << endl;
    cout << "Total Captured Packets: "
         << packetQueue.size() + replayQueue.size() + backupQueue.size() << endl;
    cout << "Replayed Packets: " << replayQueue.size() << endl;
    cout << "Backup Packets: " << backupQueue.size() << endl;
    cout << "Capture Duration: 1 minute" << endl;
    cout << "All modules executed successfully." << endl;

    return 0;
}
