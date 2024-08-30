#include "common.h"

long long GetHighResolutionTimeMS() {
	return std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

long long GetHighResolutionTimeNS() {
	return std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void PrintLastError(DWORD error_code, const std::wstring& msg) {
	wchar_t buffer[512];

	auto dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

	auto size = FormatMessageW(
		dwFlags,
		NULL,
		error_code,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		buffer,
		sizeof(buffer),
		NULL
	);

	if (size > 0) {
		if (size >= 2 && buffer[size - 2] == L'\r' && buffer[size - 1] == L'\n') {
			buffer[size - 2] = L'\0';
		}
		if (msg.size()) {
			std::wcout << msg + L": " + buffer + L" - " + std::to_wstring(error_code) + L'\n';
		}
		else {
			std::wcout << L"Error: " + std::wstring(buffer) + L" - " + std::to_wstring(error_code) + L"\n";
		}
	}
	else {
		if (msg.size()) {
			std::wcout << msg + L": Unkonwn Error - " + std::to_wstring(error_code) + L"\n";
		}
		else {
			std::wcout << std::wstring(L"Error") + L": Unkonwn Error - " + std::to_wstring(error_code) + L"\n";
		}
	}
}