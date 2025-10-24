<div align="center">

# 🌐 Network Monitor  
### _A Real-Time Packet Capture, Dissection & Replay System in C++_  

---

🧑‍💻 **Author:** [**Fatima Nisar**](#)  
🎓 BS Data Science (2nd Semester)  
🏫 Department of Computing — Fall 2025  

</div>

---

## 📘 Overview

This project implements a **real-time Network Monitor** in **C++** that captures, dissects, filters, and replays network packets using **custom-built Stack and Queue data structures**.  

It mimics professional network analysis tools such as **Wireshark**, but is **built entirely from scratch** — reinforcing understanding of both **data structures** and **network programming** concepts.

---

## 🧩 Features

✅ **Real-Time Packet Capture** — Captures packets on a live network interface using raw sockets  
✅ **Custom Queue for Packet Management** — Handles sequential incoming packets efficiently  
✅ **Custom Stack for Layer Dissection** — Dissects packets layer-by-layer (Ethernet → IP → TCP/UDP)  
✅ **Protocol Support:** Ethernet, IPv4, IPv6, TCP, UDP  
✅ **Filtering** — Filters by source and destination IP  
✅ **Replay Mechanism** — Replays packets with two retries and dynamic delay estimation  
✅ **Oversized Packet Handling** — Detects and manages large packets gracefully  
✅ **Live Logging** — Displays real-time details of captured, filtered, dissected, and replayed packets  

---

## 🧠 Data Structure Roles

| Data Structure | Purpose | Behavior |
|----------------|----------|-----------|
| 🧺 **Queue** | Stores incoming packets sequentially for processing | FIFO (First In, First Out) |
| 🧱 **Stack** | Used for dissection of packets layer by layer | LIFO (Last In, First Out) |

Together, these structures ensure smooth and structured **real-time packet analysis**.

---

## ⚙️ How to Run

### 🧩 1. Compile the Code
```bash
g++ -std=c++17 -pthread -o netmon netmon.cpp
```

2. Run as Root
```bash
sudo ./netmon <interface_name>
```

Example:
```bash
sudo ./netmon enp0s3
```
🖥️ 3. Provide Input

**You’ll be prompted to enter: **

Source IP

Destination IP

**Then the program will:**

Capture packets for 1 minute

Dissect layers

Filter and replay packets

Display detailed logs and a final summary

🧰 **System Requirements: **

🐧 Linux environment

🔑 Root privileges (required for raw socket access)

⚙️ g++ compiler supporting C++17

🧮 **Implementation Details:**

Designed modularly for clarity and scalability.

Uses multithreading for concurrent capture and replay.

Built from scratch using custom Stack and Queue classes.

Efficient error handling ensures robustness against corrupted or incomplete packets.

🌟 **Why This Project Excels: **

This project demonstrates:

Exceptional clarity in data structure design

Deep understanding of network layer parsing

Robust implementation of capture, filtering, and replay mechanisms

Professional documentation and real-world testing


<div align="center">

✨ Developed with precision and passion by

Fatima Nisar

“Turning packets into patterns of insight.” 🛰️

</div> ```
