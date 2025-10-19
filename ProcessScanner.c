#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <stdio.h>

int main() {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Konsol rengi için handle al
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Snapshot al
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("[-] Snapshot alınamadı. Hata kodu:%lu\n", GetLastError());
        return 1;
    }

    // İlk prosesi al ve döngü başlat
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Process'e eriş
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                WCHAR path[MAX_PATH] = L"";

                // Bellek bilgisi çek
                SIZE_T ramUsage = 0;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    ramUsage = pmc.WorkingSetSize;
                }

                // Executable path al
                GetModuleFileNameEx(hProcess, NULL, path, MAX_PATH);

                // Konsol rengi: 100 MB üzeri ise kırmızı, değilse yeşil
                if (ramUsage > (100 * 1024 * 1024)) {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                }
                else {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                }

                // Yazdır
                wprintf(L"[PID:%6u] %-25ws RAM: %.2f MB\n", pe32.th32ProcessID, pe32.szExeFile, ramUsage / (1024.0 * 1024));
                wprintf(L"  Path: %s\n\n", path);

                // Renk sıfırla
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                CloseHandle(hProcess);
            }
            else {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
                wprintf(L"[PID:%6u] %ws - (Erişim reddedildi)\n\n", pe32.th32ProcessID, pe32.szExeFile);
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    else {
        printf("[-] Process bilgisi alınamadı.\n");
    }

    CloseHandle(hSnapshot);
    return 0;
}
