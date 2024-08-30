# Sockets Monitor

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)
![WinAPI](https://img.shields.io/badge/Tech-WinAPI-orange.svg)
![License: GPLv2.0](https://img.shields.io/badge/License-GPLv2.0-yellow.svg)
![GitHub pull requests](https://img.shields.io/github/issues-pr/StupNum/Sockets-Monitor)
![GitHub forks](https://img.shields.io/github/forks/StupNum/Sockets-Monitor?style=social)

**Sockets-Monitor** is a project for monitoring the network socket pool. Displays the client port of the last received packet, the number of port changes since monitoring began, the number of port changes per second, and the number of packets received per second.

## Features
- High-speed packet processing in asynchronous mode using completion port
- Color indication showing packet flow and port changes
- Adaptive resizing of the console based on the number of ports and additional socket information, according to 16:9 aspect ratio

## Demo
![Demo gif display nothing](docs/demo_display_nothing.gif)
![Demo gif display all](docs/demo_display_all.gif)

## Using
Set the constants for the number of sockets to monitor, the initial port from which the socket pool will be allocated. Then create an object of ConsoleManager class, optionally set flags to display additional information (more details below). 

```cpp
constexpr int port_count = 256;
constexpr int start_port_number = 20000;
ConsoleManager console_manager(port_count, ConsoleManager::DisplayNothing);
```

 Then create an object of the NetworkManager class, with the same parameters, and pass the mutex of the console_manager object through the GetMutex getter. Next, set the network_manager object to a shared vector that stores the port records of received packets, so that network_manager can write data to them.
```cpp
NetworkManager network_manager(port_count, start_port_number, console_manager.GetMutex());
network_manager.SetPortPairs(console_manager.GetPortPairs());
```

Next, initialize the open ports entries in the console_manager object using the GetOpenedSocketsPorts getter of the network_manager object.
```cpp
console_manager.InitEntries(network_manager.GetOpenedSocketsPorts());
```

Next, start ListenSockets of the network_manager object in a separate thread, and call the console update function of the console_manager in the main thread (or also in a separate thread, if required)
```cpp
std::thread network_manager_thread(&NetworkManager::ListenSockets, &network_manager);
network_manager_thread.detach();

console_manager.StartConsoleUpdateLoop();
```

## Color indication
Color indication was made for easy perception. When a packet is received, the client port number lights up green for 1 second. When the port is changed, the color indication works as follows: 
* If the port is changed 1 time in the last 5 seconds, the host port number will light up yellow,
* if it is changed more than once, the number will light up red,
* if it is more than 10 times, the number will light up purple.

Port change control period is 5 seconds.

## TODO

## Getting Started
1. Clone the repository
2. Build with C++17 or higher
3. Run the executable

## Contributing
Bug reports and/or pull requests are welcome

## License
This project is licensed under the [GPL License, Version 2.0](https://opensource.org/license/gpl-2-0)
