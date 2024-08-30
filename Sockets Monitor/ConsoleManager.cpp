#include "ConsoleManager.h"



ConsoleManager::ConsoleManager(int port_count, unsigned int display_mode) :
    ports_entries_count_(port_count),
    display_mode_(display_mode) {
    UpdatePortEntryDisplaySize();
    InitConsole();
}

void ConsoleManager::StartConsoleUpdateLoop() {
    update_loop_flag_ = 1;
    while (update_loop_flag_) {
        auto start = GetHighResolutionTimeNS();
        while (GetHighResolutionTimeNS() - start < kFrameDelayNS) {
            ParsePortPairs();
            UpdateEntries();
            WritePortsEntriesToBuffer();
            UpdateConsoleBuffer();
        }
    }
}

void ConsoleManager::StopConsoleUpdateLoop() {
    update_loop_flag_ = 0;
}

std::vector<std::pair<int, int>>* ConsoleManager::GetPortPairs() {
    return &port_pairs_;
}

std::mutex& ConsoleManager::GetMutex() {
    return mutex_;
}

bool ConsoleManager::InitConsole() {
    console_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);

    SetCurrentConsoleFontEx(console_handle_, 0, &console_font_);
    UpdateConsoleCharachterAspectRatio();
    SetOptimalSize();

    buffer_char_info_array_size_ = static_cast<size_t>(width_) * static_cast<size_t>(height_);
    console_window_size_rect_.Left = 0;
    console_window_size_rect_.Top = 0;
    console_window_size_rect_.Right = width_ - 1;
    console_window_size_rect_.Bottom = height_ - 1;

    COORD big_console_buffer_size_{ 2000, 2000 };
    if (!SetConsoleScreenBufferSize(console_handle_, big_console_buffer_size_)) {
        PRINT_LAST_ERROR(L"Set large console buffer size");
        return 1;
    }

    if (!SetConsoleWindowInfo(console_handle_, true, &console_window_size_rect_)) {
        PRINT_LAST_ERROR(L"Set needed console buffer size");
        return 1;
    }
    if (!SetConsoleScreenBufferSize(console_handle_, console_buffer_size_)) {
        PRINT_LAST_ERROR_DEFAULT();
        return 1;
    }
    if (!SetConsoleWindowInfo(console_handle_, true, &console_window_size_rect_)) {
        PRINT_LAST_ERROR(L"Set needed console buffer size");
        return 1;
    }

    buffer_char_info_array_ = std::make_unique<CHAR_INFO[]>(buffer_char_info_array_size_);
    ClearConsole();
    return 0;
}

void ConsoleManager::UpdatePortEntryDisplaySize() {
    int count_display_digits_count_packets =
        (display_mode_ & DisplayOptions::DisplayCountPackets) > 0 ? kCountDisplayDigitsCountPackets : 0;
    int count_display_digits_count_port_changes =
        (display_mode_ & DisplayOptions::DisplayCountPortChanges) > 0 ? kCountDisplayDigitsCountPortChanges : 0;
    int count_display_digits_count_port_changes_for_period =
        (display_mode_ & DisplayOptions::DisplayCountPortChangesForPeriod) > 0 ? kCountDisplayDigitsCountPortChangesForPeriod : 0;
    int count_display_digits_count_recieved_packets_for_second =
        (display_mode_ & DisplayOptions::DisplayCountRecievedPacketsForSecond) > 0 ? kCountDisplayDigitsCountRecievedPacketsForPeriod : 0;

    port_entry_display_size_ =
        kPortEntrySize +
        count_display_digits_count_packets +
        count_display_digits_count_port_changes +
        count_display_digits_count_port_changes_for_period +
        count_display_digits_count_recieved_packets_for_second +
        static_cast<bool>(count_display_digits_count_packets) +
        static_cast<bool>(count_display_digits_count_port_changes) +
        static_cast<bool>(count_display_digits_count_port_changes_for_period) +
        static_cast<bool>(count_display_digits_count_recieved_packets_for_second);
}

void ConsoleManager::UpdateConsoleBuffer() {
    WriteConsoleOutput(
        console_handle_,
        buffer_char_info_array_.get(),
        console_buffer_size_,
        { 0, 0 },
        &console_window_size_rect_
    );
}

void ConsoleManager::ClearConsole() {
    ClearBuffer();
    UpdateConsoleBuffer();
}

void ConsoleManager::ClearBuffer() {
    for (size_t i = 0; i < buffer_char_info_array_size_; i++) {
        buffer_char_info_array_[i].Char.UnicodeChar = L' ';
        buffer_char_info_array_[i].Attributes = ConsoleColors::TextColor::kWhite | ConsoleColors::BackgroundColor::kBgBlack;
    }
}

void ConsoleManager::UpdateConsoleCharachterAspectRatio() {
    
    CONSOLE_SCREEN_BUFFER_INFO console_screen_buffer_info{};
    GetConsoleScreenBufferInfo(console_handle_, &console_screen_buffer_info);

    // Размеры консольного окна в символах
    auto rows = console_screen_buffer_info.srWindow.Bottom - console_screen_buffer_info.srWindow.Top + 1;
    auto cols = console_screen_buffer_info.srWindow.Right - console_screen_buffer_info.srWindow.Left + 1;

    // Получаем размеры окна консоли в пикселях
    HWND handle_console_window = GetConsoleWindow();
    RECT console_window_rect;
    GetWindowRect(handle_console_window, &console_window_rect);

    // Вычисляем размеры символа в пикселях
    int console_charachter_width_px = console_window_rect.right - console_window_rect.left;
    int console_charachter_height_px = console_window_rect.bottom - console_window_rect.top;

    // Учитываем рамку окна (если есть)
    console_charachter_width_px -= GetSystemMetrics(SM_CXSIZEFRAME) * 2;
    console_charachter_height_px -= GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

    console_charachter_offset_ratio_ = (static_cast<double>(console_charachter_width_px) / cols) /
        (static_cast<double>(console_charachter_height_px) / rows);
}

void ConsoleManager::SetOptimalSize() {
    auto largest_console_window_size = GetLargestConsoleWindowSize(console_handle_);

    rows_ = ports_entries_count_;
    cols_ = 1;


    //rows, cols, aspect ratio, free port entries
    std::vector<std::tuple<int, int, double, int>> ratio_selection_cases{};


    //brute force all possible aspects ratios
    for (int cols = 1, rows = 0; cols <= ports_entries_count_; cols++) {
        rows = static_cast<int>(std::ceil(static_cast<double>(ports_entries_count_) / cols));
        double width = cols * (port_entry_display_size_ + kCountSpacesBetweenCols) - kCountSpacesBetweenCols;
        double height = rows * 2;
        double aspect_ratio = width / height * console_charachter_offset_ratio_;
        int free_avaiable_port_entries_count = rows * cols - ports_entries_count_;
        ratio_selection_cases.push_back({ rows, cols, aspect_ratio, free_avaiable_port_entries_count });
    }


    std::sort(ratio_selection_cases.begin(), ratio_selection_cases.end(),
        [](const auto& lhs, const auto& rhs) {
            constexpr auto kBiasFactor = 0.8;
            if (std::get<2>(lhs) == std::get<2>(rhs)) {
                return std::get<3>(lhs) < std::get<3>(rhs);
            }

            double lhs_value = std::get<2>(lhs);
            double rhs_value = std::get<2>(rhs);

            // Вводим корректировку
            double lhs_diff = lhs_value < kOptimalAspectRatio ? std::abs(lhs_value - kOptimalAspectRatio) * kBiasFactor : std::abs(lhs_value - kOptimalAspectRatio);
            double rhs_diff = rhs_value < kOptimalAspectRatio ? std::abs(rhs_value - kOptimalAspectRatio) * kBiasFactor : std::abs(rhs_value - kOptimalAspectRatio);

            return lhs_diff < rhs_diff;
        });


    rows_ = std::get<0>(ratio_selection_cases[0]);
    cols_ = std::get<1>(ratio_selection_cases[0]);


    ////sort original array by aspect ratio diff with optimalAspectRatio
    //auto& ratio_selection_cases_sort_by_aspect_ratio = ratio_selection_cases;
    //
    //
    //
    //
    //
    //
    //);

    ////copy array and sort by free avaiable port entries count
    //auto ratio_selection_cases_sort_by_free_avaiable_port_entries = ratio_selection_cases;
    //std::sort(ratio_selection_cases_sort_by_free_avaiable_port_entries.begin(), ratio_selection_cases_sort_by_free_avaiable_port_entries.end(),
    //    [](const auto& lhs, const auto& rhs) {
    //        return std::get<3>(lhs) < std::get<3>(rhs);
    //    }
    //);

    ////best ratio offset have least free_avaiable_port_entries_count
    //if (std::get<0>(ratio_selection_cases_sort_by_aspect_ratio[0]) == std::get<0>(ratio_selection_cases_sort_by_free_avaiable_port_entries[0])) {
    //    rows_ = std::get<0>(ratio_selection_cases_sort_by_aspect_ratio[0]);
    //    cols_ = std::get<1>(ratio_selection_cases_sort_by_aspect_ratio[0]);
    //}

    //else {
    //    //best free_avaiable_port_entries_count rows count in first two elements of ratio_selection_cases_sort_by_aspect_ratio

    //    auto target_rows = std::get<0>(ratio_selection_cases_sort_by_free_avaiable_port_entries[0]);
    //    auto it = std::find_if(ratio_selection_cases_sort_by_aspect_ratio.begin(), ratio_selection_cases_sort_by_aspect_ratio.end(),
    //        [target_rows](const std::tuple<int, int, double, int>& element) {
    //            return std::get<0>(element) == target_rows;
    //        }
    //    );
    //    size_t index = std::distance(ratio_selection_cases_sort_by_aspect_ratio.begin(), it);

    //    if (index < 2) {
    //        rows_ = std::get<0>(ratio_selection_cases_sort_by_free_avaiable_port_entries[0]);
    //        cols_ = std::get<1>(ratio_selection_cases_sort_by_free_avaiable_port_entries[0]);
    //    }
    //    //else pick best of 2 first aspect ratios by free_avaiable_port_entries_count
    //    else {
    //        const auto& ratio_selection_case_1 = ratio_selection_cases_sort_by_aspect_ratio[0];
    //        const auto& ratio_selection_case_2 = ratio_selection_cases_sort_by_aspect_ratio[1];

    //        const auto& selected_ratio_case =
    //            (std::get<3>(ratio_selection_case_1) < std::get<3>(ratio_selection_case_2))
    //            ? (ratio_selection_case_1)
    //            : (ratio_selection_case_2);

    //        rows_ = std::get<0>(selected_ratio_case);
    //        cols_ = std::get<1>(selected_ratio_case);
    //    }
    //}


    //if (cols_ > 1) {
    //    double height = rows_ * 2 + 2;
    //    double width = cols_ * port_entry_display_size_ - kCountSpacesBetweenCols + 2;
    //    double aspect_ratio = width / height;

    //    if (aspect_ratio > kOptimalAspectRatio) {
    //        //decrease row count
    //        while ((rows_ - 1) * cols_ > ports_entries_count_) {
    //            rows_--;
    //            SetConsoleTitleW(L"HAHAHAHAHAHAHAHA");
    //        }
    //    }
    //}


    int target_width = cols_ * (port_entry_display_size_ + kCountSpacesBetweenCols) - kCountSpacesBetweenCols + 2;
    int target_height = rows_ * 2 + 1;

    if (largest_console_window_size.X - 1 < target_width || largest_console_window_size.Y - 1 < target_height) {
        console_font_.dwFontSize.Y--;
        SetCurrentConsoleFontEx(console_handle_, 0, &console_font_);
        UpdateConsoleCharachterAspectRatio();
        SetOptimalSize();
        return;
    }
    
    width_ = target_width;
    height_ = target_height;
    /*int avaiable_width = width_ - 2;
    int avaiable_height = height_ - 2;
    int max_rows_count = avaiable_height / 2;
    int max_cols_count = avaiable_width / (kCountSpacesBetweenCols + port_entry_display_size);
    int max_avaiable_port_entries_count = max_rows_count * max_cols_count;


    if (max_avaiable_port_entries_count >= ports_entries_count_) {
        int free_avaiable_port_entries_count = max_avaiable_port_entries_count - ports_entries_count_;
        if (free_avaiable_port_entries_count / max_rows_count > 0) {

        }
        int unnecessary_spaces = avaiable_width - (max_cols_count * (kCountSpacesBetweenCols + kPortEntrySize));
        extra_spaces = unnecessary_spaces / max_cols_count;
    }
    else {

    }*/
}

bool ConsoleManager::CheckConsoleSizeChange() {
    return 0;
}

void ConsoleManager::CustomizeDisplayOptions() {
    CONSOLE_SCREEN_BUFFER_INFO console_screen_buffer_info{};
    GetConsoleScreenBufferInfo(console_handle_, &console_screen_buffer_info);
    auto width = console_screen_buffer_info.srWindow.Right;
    auto height = console_screen_buffer_info.srWindow.Bottom;
}

void ConsoleManager::ParsePortPairs() {
    auto time = GetHighResolutionTimeMS();
    std::lock_guard<std::mutex> lg(mutex_);
    for (auto const& port_pair : port_pairs_) {
        auto& client_ports_and_time_vec = ports_pairs_data_[port_pair.first];
        client_ports_and_time_vec.push_back({ port_pair.second, time });
    }
    port_pairs_.clear();
}

bool ConsoleManager::InitEntries(std::vector<int> opened_sockets_ports) {
    for (auto const& host_socket_port : opened_sockets_ports) {
        ports_pairs_data_[host_socket_port] = std::vector<std::pair<int, long long>>{};
        auto port_entry = PortEntry{};
        port_entry.host_port = host_socket_port;
        port_entry.client_port = -1;
        port_entry.period_port_control = 5000;
        port_entry.host_port_text_color = ConsoleColors::TextColor::kWhite;
        port_entry.client_port_text_color = ConsoleColors::TextColor::kWhite;
        port_entry.host_port_bg_color = ConsoleColors::BackgroundColor::kBgBlack;
        port_entry.client_port_bg_color = ConsoleColors::BackgroundColor::kBgBlack;
        ports_entries_.push_back(port_entry);
    }
    return 0;
}

void ConsoleManager::UpdateEntries() {
    auto time = GetHighResolutionTimeMS();

    for (auto& port_entry : ports_entries_) {
        auto& client_ports_and_time_vec = ports_pairs_data_[port_entry.host_port];
        const auto period_port_control = port_entry.period_port_control;
        auto& port_time_pairs_for_period = port_entry.port_pairs_for_period;
        if (client_ports_and_time_vec.size()) {
            port_entry.last_time_packet_recieved = client_ports_and_time_vec.back().second;
        }
        port_entry.сount_packets += static_cast<int>(client_ports_and_time_vec.size());

        // Очистка устаревших записей о изменении порта
        while (!port_time_pairs_for_period.empty() && time - port_time_pairs_for_period.front().second >= period_port_control) {
            port_entry.port_pairs_for_period.pop_front();
        }
        //очистка записей о пакетах в секунду
        while (!port_entry.recieved_packets_for_second.empty() && time - port_entry.recieved_packets_for_second.front() >= 1000) {
            port_entry.recieved_packets_for_second.pop_front();
        }
        
        int current_client_port = port_entry.client_port;
        for (auto const& port_time_pair : client_ports_and_time_vec) {
            // Добавляем в очередь только те порты, которые менялись
            if (time - port_time_pair.second <= period_port_control && port_time_pair.first != current_client_port) {
                port_time_pairs_for_period.push_back(port_time_pair);
                current_client_port = port_time_pair.first;
                port_entry.count_port_changes++;
            }
            //добавляем все пакеты, пришедшие не позднее секунды назад в очередь для отслеживания количества пакетов в секунду
            if (time - port_time_pair.second <= 1000) {
                port_entry.recieved_packets_for_second.push_back(port_time_pair.second);
            }
        }

        if (port_time_pairs_for_period.size() && port_entry.client_port != port_time_pairs_for_period.back().first) {
            if (port_entry.client_port == -1) {
                port_entry.client_port = port_time_pairs_for_period.front().first;
                port_time_pairs_for_period.pop_front();
            }
            if (port_time_pairs_for_period.size()) {
                port_entry.client_port = port_time_pairs_for_period.back().first;
            }
        }
        port_entry.count_port_changes_for_period = static_cast<int>(port_time_pairs_for_period.size());
        port_entry.count_recieved_packets_for_second = static_cast<int>(port_entry.recieved_packets_for_second.size());
        client_ports_and_time_vec.clear();

        if (GetHighResolutionTimeMS() - port_entry.last_time_packet_recieved <= 1000) {
            port_entry.client_port_text_color = ConsoleColors::TextColor::kBlack;
            port_entry.client_port_bg_color = ConsoleColors::BackgroundColor::kBgGreen;
        }
        else {
            port_entry.client_port_text_color = ConsoleColors::TextColor::kWhite;
            port_entry.client_port_bg_color = ConsoleColors::BackgroundColor::kBgBlack;
        }

        if (port_entry.count_port_changes_for_period == 0) {
            port_entry.host_port_text_color = ConsoleColors::TextColor::kWhite;
            port_entry.host_port_bg_color = ConsoleColors::BackgroundColor::kBgBlack;
        }
        if (port_entry.count_port_changes_for_period == 1) {
            port_entry.host_port_text_color = ConsoleColors::TextColor::kGreen;
            port_entry.host_port_bg_color = ConsoleColors::BackgroundColor::kBgYellow;
        }
        if (port_entry.count_port_changes_for_period > 1) {
            port_entry.host_port_text_color = ConsoleColors::TextColor::kWhite;
            port_entry.host_port_bg_color = ConsoleColors::BackgroundColor::kBgRed;
        }
        if (port_entry.count_port_changes_for_period > 10) {
            port_entry.host_port_text_color = ConsoleColors::TextColor::kWhite;
            port_entry.host_port_bg_color = ConsoleColors::BackgroundColor::kBgMagenta;
        }
    }
}

void ConsoleManager::WritePortsEntriesToBuffer() {
    std::wstring temp_string{};

    for (auto col = 0, port_entry_index = 0; col < cols_ && port_entry_index < ports_entries_count_; col++) {
        for (auto row = 0; row < rows_ && port_entry_index < ports_entries_count_; row++) {
            const auto& entry = ports_entries_[port_entry_index++];
            auto current_write_buffer_index = row * width_ * 2 + col * (port_entry_display_size_ + kCountSpacesBetweenCols) + 1 + width_;

            auto AddNumberToBuffer = [&entry, &current_write_buffer_index, &temp_string, this](
                int number_to_write,
                int max_digits_count,
                wchar_t end_symbol = L' ',
                wchar_t align_side = L'R',
                int number_color = ConsoleColors::TextColor::kWhite | ConsoleColors::BackgroundColor::kBgBlack,
                bool fill_spaces = 0) {
                    wchar_t fill_symbol = (fill_spaces) ? L' ' : kFillSymbol;
                    temp_string = std::to_wstring(number_to_write);
                    if (temp_string.size() > max_digits_count) {
                        temp_string.resize(max_digits_count);
                    }
                    else {
                        if (align_side == 'L') {
                            temp_string.append(max_digits_count - temp_string.size(), fill_symbol);
                        }
                        else {
                            temp_string.insert(0, max_digits_count - temp_string.size(), fill_symbol);
                        }
                    }
                    for (int i = 0; i < max_digits_count; i++, current_write_buffer_index++) {
                        buffer_char_info_array_[current_write_buffer_index].Char.UnicodeChar = temp_string[i];
                        buffer_char_info_array_[current_write_buffer_index].Attributes = number_color;
                    }
                    if (end_symbol != L'\0') {
                        buffer_char_info_array_[current_write_buffer_index].Char.UnicodeChar = end_symbol;
                        buffer_char_info_array_[current_write_buffer_index].Attributes = ConsoleColors::TextColor::kWhite | ConsoleColors::BackgroundColor::kBgBlack;
                        current_write_buffer_index++;
                    }
                };

            //host port
            AddNumberToBuffer(entry.host_port, kCountDisplayDigitsHostPortText, L':', L'L', entry.host_port_text_color | entry.host_port_bg_color, 1);

            //client port
            AddNumberToBuffer(entry.client_port, kCountDisplayDigitsClientPortText, L' ', L'R', entry.client_port_text_color | entry.client_port_bg_color, 1);

            //count port changes for period
            if (display_mode_ & DisplayOptions::DisplayCountPortChangesForPeriod) {
                AddNumberToBuffer(entry.count_port_changes_for_period, kCountDisplayDigitsCountPortChangesForPeriod);
            }

            //count port changes 
            if (display_mode_ & DisplayOptions::DisplayCountPortChanges) {
                AddNumberToBuffer(entry.count_port_changes, kCountDisplayDigitsCountPortChanges);
            }

            //count packets count
            if (display_mode_ & DisplayOptions::DisplayCountPackets) {
                AddNumberToBuffer(entry.сount_packets, kCountDisplayDigitsCountPackets);
            }

            if (display_mode_ & DisplayOptions::DisplayCountRecievedPacketsForSecond) {
                AddNumberToBuffer(entry.count_recieved_packets_for_second, kCountDisplayDigitsCountRecievedPacketsForPeriod, L'\0');
            }
        }
    }




    /*for (const auto& entry : ports_entries_) {
        if (row >= height_) break;

        std::wstring hostPortStr = std::to_wstring(entry.host_port);
        hostPortStr.append(5 - hostPortStr.size(), L' ');

        std::wstring clientPortStr = std::to_wstring(entry.client_port);
        clientPortStr.append(5 - clientPortStr.size(), L' ');

        std::wstring displayStr = hostPortStr + L":" + clientPortStr;

        // Вывод строки в консоль
        for (size_t i = 0; i < displayStr.size(); ++i) {
            buffer_char_info_array_[row * width_ + col + i].Char.UnicodeChar = displayStr[i];
            if (i < 5) {
                buffer_char_info_array_[row * width_ + col + i].Attributes = entry.host_port_text_color | entry.host_port_bg_color;
            }
            else if (i > 5) {
                buffer_char_info_array_[row * width_ + col + i].Attributes = entry.client_port_text_color | entry.client_port_bg_color;
            }
            else {
                buffer_char_info_array_[row * width_ + col + i].Attributes = ConsoleColors::TextColor::kWhite | ConsoleColors::BackgroundColor::kBgBlack;
            }
        }

        // Увеличиваем row, чтобы добавить пробел между записями
        row += 2;
        if (row >= height_) {
            row = 0;
            col += 20;
        }
    }*/
}

ConsoleManager::~ConsoleManager() {}






