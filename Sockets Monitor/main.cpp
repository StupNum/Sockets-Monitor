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
	constexpr int port_count = 256;
	constexpr int start_port_number = 20000;
	ConsoleManager console_manager(port_count, ConsoleManager::DisplayNothing);

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























//#include <windows.h>
//#include <iostream>
//
//// Функция APC, которая будет добавлена в очередь APC
//void CALLBACK APCProcedure(ULONG_PTR dwParam) {
//    DWORD threadID = GetCurrentThreadId();
//    std::cout << "APC Procedure started in Thread ID: " << threadID << std::endl;
//
//    // Бесконечный цикл
//    while (true) {
//        std::cout << "APC running in Thread ID: " << threadID << std::endl;
//        Sleep(100); // Чтобы не перегружать вывод
//    }
//}
//
//DWORD WINAPI ThreadFunction(LPVOID lpParam) {
//    DWORD threadID = GetCurrentThreadId();
//    std::cout << "Thread ID: " << threadID << " is running and waiting for APC..." << std::endl;
//
//    // Переводим поток в alterable состояние на 1 секунду
//    SleepEx(1000, 0);
//
//    std::cout << "Thread ID: " << threadID << " finished waiting." << std::endl;
//    return 0;
//}
//
//int main() {
//    // Создание потока
//    HANDLE hThread = GetCurrentThread();
//
//
//    // Добавление APC в очередь потока
//    QueueUserAPC(APCProcedure, hThread, NULL);
//
//    // Ожидание завершения потока
//    
//    SleepEx(2000, 1);
//    // Закрытие дескриптора потока
//    CloseHandle(hThread);
//
//    return 60;
//}



//#include <windows.h>
//#include <iostream>
//#include <mutex>
//#include <chrono>
//
//int flag;
//std::mutex M{};
//decltype(std::chrono::high_resolution_clock::now()) start;
//auto a = INADDR_ANY;
//int ms = 800;
//DWORD WINAPI ThreadFunction(LPVOID lpParameter) {
//    DWORD threadID = GetCurrentThreadId();
//
//    while (flag)
//        
//        if (std::chrono::duration_cast<std::chrono::milliseconds>
//            (std::chrono::high_resolution_clock::now() - start).count() > ms) {
//            flag = 0;
//        }
//        // Смена контекста на другой поток
//        if (SwitchToThread() == 0) {
//            //std::lock_guard<std::mutex> lock(M);
//            // Если SwitchToThread вернула 0, значит контекст не был переключен
//            //std::cout << "Thread ID " << threadID << " could not switch context!" << std::endl;
//        }
//        
//        //std::cout << "WHY(((((\n";
//    return 0;  // Хотя поток никогда не завершится, добавляем return для соответствия сигнатуре
//}
//
//int main() {
//    constexpr int count = 25;
//    while (true) {
//        flag = 1;
//        start = std::chrono::high_resolution_clock::now();
//        // Устанавливаем приоритет текущего процесса на минимальный
//        HANDLE hProcess = GetCurrentProcess();
//        if (!SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS)) {
//            std::cerr << "Failed to set process priority, error: " << GetLastError() << std::endl;
//            return 1;
//        }
//        if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL)) {
//            std::cerr << "Failed to set thread priority, error: " << GetLastError() << std::endl;
//            return 1;
//        }
//        // Создаем count потоков с минимальным приоритетом
//        HANDLE threads[count];
//        for (int i = 0; i < count; ++i) {
//            threads[i] = CreateThread(
//                NULL,               // Атрибуты безопасности
//                0,                  // Начальный размер стека
//                ThreadFunction,     // Функция потока
//                NULL,               // Аргумент функции потока
//                0,                  // Параметры создания
//                NULL);              // Идентификатор потока
//
//            if (!threads[i]) {
//                std::cerr << "Failed to create thread " << i << ", error: " << GetLastError() << std::endl;
//                return 1;
//            }
//            
//            // Устанавливаем минимальный приоритет для созданного потока
//            if (!SetThreadPriority(threads[i], THREAD_PRIORITY_TIME_CRITICAL)) {
//                std::cerr << "Failed to set thread priority, error: " << GetLastError() << std::endl;
//                return 1;
//            }
//        }
//
//        //Sleep(1203910923091);
//        // Ожидаем завершения всех потоков (что, конечно, никогда не произойдет в данном примере)
//        WaitForMultipleObjectsEx(count, threads, TRUE, INFINITE, 0);
//        //exit(111);
//        SleepEx(700, 0);
//    }
//    return 666;
//    //// Закрываем дескрипторы потоков
//    //for (int i = 0; i < count; ++i) {
//    //    CloseHandle(threads[i]);
//    //}
//
//}
