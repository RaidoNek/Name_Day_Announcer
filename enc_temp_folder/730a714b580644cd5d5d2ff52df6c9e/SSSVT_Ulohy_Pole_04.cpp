﻿#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <Windows.h>
#include <string>

#pragma warning(disable : 4996)

bool hasWindows() {
#if defined(WIN32) || defined(WIN32) || defined(__WIN32) && !defined(__CYGWIN_)
    return true;
#else
    return false;
#endif
}

std::wstring utf8_to_wide(const std::string& str)
{
    int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
    return wstr;
}

std::chrono::system_clock::duration duration_since_midnight() {
    auto now = std::chrono::system_clock::now();

    time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm* date = std::localtime(&tnow);
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;
    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));

    return now - midnight;
}

int get_hours() {
    std::cout << "Enter a hour (0-23, not AM/PM): ";
    std::string hour_s;
    std::cin >> hour_s;
    
    return std::stoi(hour_s);
}

int get_minutes() {
    std::cout << "Enter a minute (0-59): ";
    std::string minutes_s;
    std::cin >> minutes_s;

    return std::stoi(minutes_s);
}

std::string get_country() {
    //declare supported country codes
    const std::string codes[] = {
        "at",
        "bg",
        "cz",
        "de",
        "dk",
        "ee",
        "es",
        "fi",
        "fr",
        "gr",
        "hr",
        "hu",
        "it",
        "lt",
        "lv",
        "pl",
        "ru",
        "se",
        "sk",
        "us"
    };

    //print info messages
    std::cout << "Hello, first enter country code for which you want to announce name days" << "\n";
    std::cout << "Supported country codes are: ";

    //declare length, dynamically show all supported country codes
    short length = (sizeof(codes) / sizeof(codes[0]));
    for (short i = 0; i < length; i++) {
        std::cout << codes[i] << ((i == (length - 1)) ? "" : " | ");
    }

    //let user to enter code
    std::cout << "\n";
    std::cout << "Enter country code: ";

    //get country code and validate
    std::string country_s;
    std::cin >> country_s;

    for (short i = 0; i < length; i++) {
        if (strcmp(country_s.c_str(), codes[i].c_str()) == 0) {
            return country_s;
        } 
    }

    //User entered invalid country code
    std::cout << "That's not a supported country code" << "\n";
    for (short i = 3; i > 0; i--) {
        std::cout << "Reseting app in " << i << "..." << "\n";
        Sleep(1000);
    }
    system(hasWindows() ? "cls" : "clear");
    get_country();
}

void Run() {
    //get country code for name day
    std::string country_code = get_country();

    //send request
    cpr::Response r = cpr::Post(cpr::Url{ "https://nameday.abalin.net/today" }, cpr::Parameters{ {"country", country_code} });
    std::string response_text = r.text;

    //parse json request response
    nlohmann::json parsed = nlohmann::json::parse(response_text, nullptr, false);

    //discard check
    if (parsed.is_discarded()) exit(0);

    //declare variables
    short u_hours = 0;
    short u_minutes = 0;

    //get announce time
    u_hours = get_hours();
    u_minutes = get_minutes();

    //print info
    std::cout << "PAUSE - Show Window Again" << "\n";
    std::cout << "DEL - Reset app and enter time again (only if u have window showed)" << "\n";

    //print hide time remaining
    for (short i = 5; i > 0; i--) {
        std::cout << "Hiding window in " << i << "..." << "\n";
        Sleep(1000);
    }

    //hide console window
    bool show = false;
    HWND window;
    AllocConsole();
    window = GetConsoleWindow();
    ShowWindow(window, show);

    while (true) {
        //get time since midnight
        auto since_midnight = duration_since_midnight();
        auto hours = std::chrono::duration_cast<std::chrono::hours>(since_midnight);
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(since_midnight - hours);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(since_midnight - hours - minutes);

        //if user clicks on PAUSE button - shows up window
        if (GetAsyncKeyState(VK_PAUSE) & 0x13) {
            show = !show;
            ShowWindow(window, show);
            std::cout << "Welcome back, would you like to reset announce time?" << "\n";
        }

        //reset announce time if window is opened
        if ((GetAsyncKeyState(VK_DELETE) & 0x13) && window) {
            for (short i = 5; i > 0; i--) {
                std::cout << "Reseting announce time in " << i << "..." << "\n";
                Sleep(1000);
            }
            system(hasWindows() ? "cls" : "clear");
            Run();
            break;
        }

        //checking if current time equals users announce time every second
        if (hours.count() == u_hours && minutes.count() == u_minutes && seconds.count() == 0) {
            for (short i = 0; const auto & var : parsed) {
                if (i == 0) {
                    //get value
                    const std::string value = var["namedays"][country_code].get<std::string>();
                    const bool contains = (value.find(",") != std::string::npos);

                    //get name... if valid -> print name, if not -> print "No one"
                    const std::string name = ((value == "n\/a") ? "No one" : value);

                    //get spelling -> if str contains , -> it definitely contains multiple names
                    const std::string spelling = ((contains) ? " have" : " has");
                    const std::string spelling2 = ((contains) ? "them" : "her/him");

                    const std::string msg = name + spelling + " a name day today, wish " + spelling2 + " luck";
                    MessageBoxW(0, utf8_to_wide(msg).c_str(), L"name day announcer", 0);

                    i++;
                }
                break;
            }
        }       
        Sleep(1000);
    }
}

int main()
{
    //call main function
    Run();

    return 0;
}