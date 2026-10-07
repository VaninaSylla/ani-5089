#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <vector>

struct MouseDelta { int dx, dy; };

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_INPUT) {
        UINT size = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
        if (size == 0) return 0;
        std::vector<BYTE> buffer(size);
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) == size) {
            RAWINPUT* raw = (RAWINPUT*)buffer.data();
            if (raw->header.dwType == RIM_TYPEMOUSE) {
                int dx = raw->data.mouse.lLastX;
                int dy = raw->data.mouse.lLastY;
                if (dx != 0 || dy != 0) {
                    std::vector<MouseDelta>* deltas = (std::vector<MouseDelta>*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    if (deltas) deltas->push_back({dx, dy});
                }
            }
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void RegisterRawInput(HWND hWnd) {
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hWnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int show) {
    const wchar_t* cls = L"AccumulateurTest";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = cls;
    wc.cbWndExtra = sizeof(void*);
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(0, cls, L"Accumulateur Test - Bougez la souris, fermez pour finir", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, hInst, nullptr);

    std::vector<MouseDelta>* deltas = new std::vector<MouseDelta>();
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)deltas);

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hWnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    ShowWindow(hWnd, show);

    // Phase 1: avant.txt - BUG: ne garde que le dernier delta
    std::ofstream out("avant.txt");
    MSG msg{};
    bool running = true;

    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running) break;

        std::vector<MouseDelta>* deltas = (std::vector<MouseDelta>*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        int dx = 0, dy = 0;
        if (deltas && !deltas->empty()) {
            dx = deltas->back().dx;
            dy = deltas->back().dy;
            deltas->clear();
        }
        out << dx << " " << dy << "\n";
        Sleep(16);
    }
    out.close();

    // Phase 2: apres.txt - CORRECT: accumule tous les deltas, reset à zéro chaque frame
    std::ofstream out2("apres.txt");
    running = true;

    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!running) break;

        std::vector<MouseDelta>* deltas = (std::vector<MouseDelta>*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        long long frame_dx = 0, frame_dy = 0;
        if (deltas) {
            for (auto& d : *deltas) { frame_dx += d.dx; frame_dy += d.dy; }
            deltas->clear();
        }
        out2 << frame_dx << " " << frame_dy << "\n";
        Sleep(16);
    }
    out2.close();

    delete deltas;
    return 0;
}