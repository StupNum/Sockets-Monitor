#include "ConsoleManager.h"
#include "NetworkManager.h"

#include <thread>



int main() {
	//for (int i = 500; i < 10000; i++) {
	//	int port_count = i;
	//	constexpr int start_port_number = 20000;
	//	ConsoleManager console_manager(port_count, ConsoleManager::DisplayAll);

	//	NetworkManager network_manager(port_count, start_port_number, console_manager.GetMutex());
	//	network_manager.SetPortPairs(console_manager.GetPortPairs());
	//	console_manager.InitEntries(network_manager.GetOpenedSocketsPorts());
	//	std::thread network_manager_thread(&NetworkManager::ListenSockets, &network_manager);
	//	network_manager_thread.detach();
	//	//network_manager_thread.~thread();
	//	std::thread console_manager_thread(&ConsoleManager::StartConsoleUpdateLoop, &console_manager);
	//	console_manager_thread.detach();
	//	/*std::thread CM_th(&ConsoleManager::StartConsoleUpdateLoop, &console_manager);
	//	CM_th.detach();*/
	//	Sleep(300);
	//	network_manager.StopListenSockets();
	//	console_manager.StopConsoleUpdateLoop();
	//	Sleep(50);
	//}
	constexpr int port_count = 100;
	constexpr int start_port_number = 20000;
	ConsoleManager console_manager(port_count, ConsoleManager::DisplayAll);

	NetworkManager network_manager(port_count, start_port_number, console_manager.GetMutex());
	network_manager.SetPortPairs(console_manager.GetPortPairs());
	console_manager.InitEntries(network_manager.GetOpenedSocketsPorts());
	std::thread network_manager_thread(&NetworkManager::ListenSockets, &network_manager);
	network_manager_thread.detach();
	//network_manager_thread.~thread();
	console_manager.StartConsoleUpdateLoop();
	/*std::thread CM_th(&ConsoleManager::StartConsoleUpdateLoop, &console_manager);
	CM_th.detach();*/
	return 0;
}























