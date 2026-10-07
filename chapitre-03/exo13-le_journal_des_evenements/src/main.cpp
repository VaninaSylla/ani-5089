#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>

std::ofstream g_logFile;
std::chrono::steady_clock::time_point g_startTime;

void LogEvent(const char* type) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_startTime).count();
    double seconds = elapsed / 1000.0;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << seconds << " " << type;
    g_logFile << oss.str() << "\n";
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            LogEvent("touche");
            break;
        case WM_KEYUP:
            LogEvent("touche");
            break;
        case WM_MOUSEMOVE:
            LogEvent("souris");
            break;
        case WM_MOUSEWHEEL:
            LogEvent("molette");
            break;
        case WM_SIZE:
            LogEvent("redimensionnement");
            break;
        case WM_SETFOCUS:
            LogEvent("entree_pointeur");
            break;
        case WM_KILLFOCUS:
            LogEvent("sortie_pointeur");
            break;
        case WM_MOUSELEAVE:
            LogEvent("sortie_pointeur");
            break;
        case WM_MOUSEHOVER:
            LogEvent("entree_pointeur");
            break;
        case WM_SETCURSOR:
            break;
        case WM_NCHITTEST:
            break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int show) {
    const wchar_t* cls = L"JournalEvents";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = cls;
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(0, cls, L"Journal Evenements - 1 minute", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hInst, nullptr);
    ShowWindow(hWnd, show);
    UpdateWindow(hWnd);

    // Track mouse hover/leave
    TRACKMOUSEEVENT tme{};
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_HOVER | TME_LEAVE;
    tme.hwndTrack = hWnd;
    tme.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&tme);

    g_logFile.open("journal.txt");
    g_startTime = std::chrono::steady_clock::now();

    MSG msg{};
    auto endTime = std::chrono::steady_clock::now() + std::chrono::minutes(1);
    
    while (std::chrono::steady_clock::now() < endTime) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { g_logFile.close(); return 0; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(1);
    }
    
    g_logFile.close();
    return 0;
}