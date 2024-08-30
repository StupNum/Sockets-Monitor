#ifndef CONSOLE_MANAGER_H
#define CONSOLE_MANAGER_H

#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#include <windows.h>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <deque>
#include <string>
#include <mutex>

#include "common.h"


struct PortEntry {
	int host_port;
	int client_port;

	int count_port_changes;
	int count_port_changes_for_period;
	int count_recieved_packets_for_second;
	int ñount_packets;
	long long period_port_control;
	long long last_time_packet_recieved;

	int host_port_text_color;
	int client_port_text_color;
	int host_port_bg_color;
	int client_port_bg_color;
	std::deque <std::pair<int, long long>> port_pairs_for_period;
	std::deque <long long> recieved_packets_for_second;
};

struct ConsoleColors {
	enum TextColor {
		kBlack = 0,
		kBlue = FOREGROUND_BLUE,
		kGreen = FOREGROUND_GREEN,
		kCyan = FOREGROUND_GREEN | FOREGROUND_BLUE,
		kRed = FOREGROUND_RED,
		kMagenta = FOREGROUND_RED | FOREGROUND_BLUE,
		kYellow = FOREGROUND_RED | FOREGROUND_GREEN,
		kWhite = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		kIntenseBlue = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		kIntenseGreen = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		kIntenseCyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		kIntenseRed = FOREGROUND_RED | FOREGROUND_INTENSITY,
		kIntenseMagenta = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		kIntenseYellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		kIntenseWhite = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	};

	enum BackgroundColor {
		kBgBlack = 0,
		kBgBlue = BACKGROUND_BLUE,
		kBgGreen = BACKGROUND_GREEN,
		kBgCyan = BACKGROUND_GREEN | BACKGROUND_BLUE,
		kBgRed = BACKGROUND_RED,
		kBgMagenta = BACKGROUND_RED | BACKGROUND_BLUE,
		kBgYellow = BACKGROUND_RED | BACKGROUND_GREEN,
		kBgWhite = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
		kIntenseBgBlue = BACKGROUND_BLUE | BACKGROUND_INTENSITY,
		kIntenseBgGreen = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
		kIntenseBgCyan = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
		kIntenseBgRed = BACKGROUND_RED | BACKGROUND_INTENSITY,
		kIntenseBgMagenta = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
		kIntenseBgYellow = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
		kIntenseBgWhite = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	};
};



class ConsoleManager {
public:
	enum DisplayOptions {
		DisplayNothing = 0,
		DisplayCountPortChanges = 1,
		DisplayCountPortChangesForPeriod = 1 << 1,
		DisplayCountPackets = 1 << 2,
		DisplayCountRecievedPacketsForSecond = 1 << 3,
		DisplayAll = 1 | 1 << 1 | 1 << 2 | 1 << 3
	};
    ConsoleManager(int port_count, unsigned int display_mode_ = DisplayOptions::DisplayNothing);
    void StartConsoleUpdateLoop();
    std::vector<std::pair<int, int>>* GetPortPairs();
    std::mutex& GetMutex();
	bool InitEntries(std::vector<int> opened_sockets_ports);
	void StopConsoleUpdateLoop();
	~ConsoleManager();

private:

    bool InitConsole();
    void ClearConsole();
    void ClearBuffer();
    void SetOptimalSize();
	bool CheckConsoleSizeChange();
    void ParsePortPairs();
    void UpdateConsoleBuffer();
	void UpdateEntries();
	void WritePortsEntriesToBuffer();
	void UpdatePortEntryDisplaySize();
	void UpdateConsoleCharachterAspectRatio();
	void CustomizeDisplayOptions();

    static constexpr int kTargetFPS = 60;
    static constexpr int kFrameDelayNS = 1'000'000'000 / kTargetFPS;
	static constexpr int kCountSpacesBetweenCols = 6;
	static constexpr int kCountSpacesBetweenRows = 1;

	static constexpr int kCountDisplayDigitsHostPortText = 5;
	static constexpr int kCountDisplayDigitsClientPortText = 5;
	static constexpr int kCountDisplayDigitsCountPortChanges = 4;
	static constexpr int kCountDisplayDigitsCountPortChangesForPeriod = 4;
	static constexpr int kCountDisplayDigitsCountPackets = 5;
	static constexpr int kCountDisplayDigitsCountRecievedPacketsForPeriod = 4;

	static constexpr int kCountPackets = 4;
	static constexpr int kInitWidth = 200; 
	static constexpr int kInitHeight = 81;
	static constexpr int kPortEntrySize = 11;
	static constexpr double kOptimalAspectRatio = 16. / 9.;
	static constexpr double kMaxAspectRatioDeviation = 0.1;
	static constexpr wchar_t kFillSymbol = L'·';

	unsigned int display_mode_{};
	int port_entry_display_size_{};
	double console_charachter_offset_ratio_{};
	int rows_{};
	int cols_{};
    HANDLE console_handle_{};
	CONSOLE_FONT_INFOEX console_font_{
		sizeof(CONSOLE_FONT_INFOEX),
		0,
		COORD{0, 18},
		0,
		FW_NORMAL,
		L"Consolas"
	};
    COORD console_buffer_size_{kInitWidth, kInitHeight};
    short& width_ = console_buffer_size_.X;
    short& height_ = console_buffer_size_.Y;
    SMALL_RECT console_window_size_rect_{};
    std::unique_ptr<CHAR_INFO[]> buffer_char_info_array_{};
    size_t buffer_char_info_array_size_{};

    int ports_entries_count_{};
    std::vector<PortEntry> ports_entries_{};
    std::vector<std::pair<int, int>> port_pairs_{};
    std::unordered_map<int, std::vector<std::pair<int, long long>>> ports_pairs_data_{};
    std::mutex mutex_;
	bool update_loop_flag_{};
};
#endif // CONSOLE_MANAGER_H