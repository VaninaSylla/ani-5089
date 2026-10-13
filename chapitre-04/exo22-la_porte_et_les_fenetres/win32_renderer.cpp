// win32_renderer.cpp — Implémentation Win32 software renderer

#include "win32_renderer.h"
#include <cmath>
#include <algorithm>

namespace nkentseu {

// ============================================================================
// Window Implementation
// ============================================================================

struct Window::Impl {
    HWND hwnd = nullptr;
    HINSTANCE hinst = nullptr;
    uint32_t width = 1280, height = 720;
    bool open = false;
    WNDCLASSEXW wc = {};
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    if (msg == WM_SIZE) {
        // Handle resize
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

Window::Window() : mImpl(std::make_unique<Impl>()) {}
Window::Window(const WindowConfig& config) : mImpl(std::make_unique<Impl>()) { Create(config); }
Window::~Window() { Close(); }

bool Window::Create(const WindowConfig& config) {
    mImpl->hinst = GetModuleHandleW(nullptr);
    mImpl->width = config.width;
    mImpl->height = config.height;

    mImpl->wc.cbSize = sizeof(WNDCLASSEXW);
    mImpl->wc.lpfnWndProc = WndProc;
    mImpl->wc.hInstance = mImpl->hinst;
    mImpl->wc.lpszClassName = L"ExerciseWindow";
    mImpl->wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    mImpl->wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClassExW(&mImpl->wc);

    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!config.resizable) style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    
    RECT rc = {0, 0, (LONG)config.width, (LONG)config.height};
    AdjustWindowRect(&rc, style, FALSE);
    
    std::wstring wtitle(config.title, config.title + strlen(config.title));
    mImpl->hwnd = CreateWindowExW(0, L"ExerciseWindow", wtitle.c_str(),
        style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right-rc.left, rc.bottom-rc.top,
        nullptr, nullptr, mImpl->hinst, nullptr);
    
    if (!mImpl->hwnd) return false;
    mImpl->open = true;
    return true;
}

void Window::Close() {
    if (mImpl->hwnd) { DestroyWindow(mImpl->hwnd); mImpl->hwnd = nullptr; }
    mImpl->open = false;
}

bool Window::IsOpen() const { return mImpl->open; }
bool Window::IsValid() const { return mImpl->hwnd != nullptr; }
void Window::SetVisible(bool v) { if (mImpl->hwnd) ShowWindow(mImpl->hwnd, v ? SW_SHOW : SW_HIDE); }
void Window::SetTitle(const char* t) { if (mImpl->hwnd) SetWindowTextA(mImpl->hwnd, t); }
uint32_t Window::GetWidth() const { return mImpl->width; }
uint32_t Window::GetHeight() const { return mImpl->height; }
HWND Window::GetHandle() const { return mImpl->hwnd; }

// ============================================================================
// Renderer3D Implementation
// ============================================================================

Renderer3D::Renderer3D() : mImpl(std::make_unique<Impl>()) {}
Renderer3D::Renderer3D(Window& w, const Renderer3DConfig& c) : mImpl(std::make_unique<Impl>()) { Create(w, c); }
Renderer3D::~Renderer3D() { Shutdown(); }

static void EnsureBitmap(Renderer3D::Impl* impl) {
    if (!impl->window || !impl->window->IsValid()) return;
    RECT rc; GetClientRect(impl->window->GetHandle(), &rc);
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    if (impl->hbmp && impl->bmpWidth == w && impl->bmpHeight == h) return;
    if (impl->hbmp) { DeleteObject(impl->hbmp); impl->hbmp = nullptr; }
    if (impl->hdc) { DeleteDC(impl->hdc); impl->hdc = nullptr; }
    
    impl->bmpWidth = w; impl->bmpHeight = h;
    
    BITMAPINFO bmi = {}; bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w; bmi.bmiHeader.biHeight = -h; // top-down
    bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    impl->hdc = CreateCompatibleDC(nullptr);
    impl->hbmp = CreateDIBSection(impl->hdc, &bmi, DIB_RGB_COLORS, &impl->bits, nullptr, 0);
    SelectObject(impl->hdc, impl->hbmp);
}

static void ClearFramebuffer(Renderer3D::Impl* impl) {
    uint32_t* pixels = (uint32_t*)impl->bits;
    int count = impl->bmpWidth * impl->bmpHeight;
    for (int i = 0; i < count; ++i) pixels[i] = impl->clearColor;
}

static void DrawLine(Renderer3D::Impl* impl, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1-x0), dy = -abs(y1-y0);
    int sx = x0<x1?1:-1, sy = y0<y1?1:-1, err = dx+dy;
    uint32_t* pix = (uint32_t*)impl->bits;
    while (true) {
        if (x0>=0 && x0<impl->bmpWidth && y0>=0 && y0<impl->bmpHeight)
            pix[y0*impl->bmpWidth + x0] = color;
        if (x0==x1 && y0==y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void ProjectPoint(const Mat4f& vp, const Vec3f& w, int sw, int sh, Vec2f& out) {
    Vec3f clip = vp * w;
    if (clip.z == 0) { out = {0,0}; return; }
    float invW = 1.0f/clip.z;
    float ndcX = clip.x*invW, ndcY = clip.y*invW;
    out.x = (ndcX+1)*0.5f*sw;
    out.y = (1-ndcY)*0.5f*sh;
}

bool Renderer3D::Create(Window& w, const Renderer3DConfig& c) {
    mImpl->window = &w; mImpl->config = c;
    if (!w.IsValid()) return false;
    EnsureBitmap(mImpl.get());
    return true;
}

void Renderer3D::Shutdown() {
    if (mImpl) {
        if (mImpl->hbmp) DeleteObject(mImpl->hbmp);
        if (mImpl->hdc) DeleteDC(mImpl->hdc);
        mImpl.reset();
    }
}

bool Renderer3D::IsValid() const { return mImpl && mImpl->window && mImpl->window->IsValid(); }

GraphicsApi Renderer3D::GetApi() const { return GraphicsApi::SOFTWARE; }
const char* Renderer3D::GetApiName() const { return "Software"; }

void Renderer3D::BeginFrame(uint32_t clearColor) {
    mImpl->clearColor = clearColor;
    mImpl->drawCalls.clear();
    mStats.drawCalls = 0; mStats.triangles = 0;
    EnsureBitmap(mImpl.get());
    ClearFramebuffer(mImpl.get());
}

void Renderer3D::SetScene(const SceneContext& s) { mImpl->scene = s; }
void Renderer3D::Submit(const DrawCall3D& d) { mImpl->drawCalls.push_back(d); mStats.drawCalls++; if(d.meshHandle==1)mStats.triangles+=12;else if(d.meshHandle==2)mStats.triangles+=2; }

void Renderer3D::Execute() {
    auto& impl = *mImpl;
    Mat4f vp = impl.scene.projMatrix * impl.scene.viewMatrix;
    
    auto drawAABB = [&](const AABB& aabb, uint32_t color) {
        if (!aabb.IsValid()) return;
        Vec3f corners[8] = {
            {aabb.min.x, aabb.min.y, aabb.min.z}, {aabb.max.x, aabb.min.y, aabb.min.z},
            {aabb.max.x, aabb.max.y, aabb.min.z}, {aabb.min.x, aabb.max.y, aabb.min.z},
            {aabb.min.x, aabb.min.y, aabb.max.z}, {aabb.max.x, aabb.min.y, aabb.max.z},
            {aabb.max.x, aabb.max.y, aabb.max.z}, {aabb.min.x, aabb.max.y, aabb.max.z}
        };
        Vec2f proj[8];
        for (int i=0;i<8;++i) ProjectPoint(vp, corners[i], impl.bmpWidth, impl.bmpHeight, proj[i]);
        int edges[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
        for (auto& e : edges) DrawLine(&impl, (int)proj[e[0]].x, (int)proj[e[0]].y, (int)proj[e[1]].x, (int)proj[e[1]].y, color);
    };
    
    for (auto& dc : impl.drawCalls) {
        uint32_t color = ((uint32_t)(dc.tint.x*255)<<24) | ((uint32_t)(dc.tint.y*255)<<16) | ((uint32_t)(dc.tint.z*255)<<8) | 255;
        drawAABB(dc.aabb, color);
    }
}

void Renderer3D::Present() {
    if (!mImpl->window || !mImpl->window->IsValid()) return;
    HWND hwnd = mImpl->window->GetHandle();
    HDC wdc = GetDC(hwnd);
    
    BITMAPINFOHEADER bih = {};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = mImpl->bmpWidth;
    bih.biHeight = -mImpl->bmpHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    
    StretchDIBits(wdc, 0, 0, mImpl->bmpWidth, mImpl->bmpHeight, 0, 0, mImpl->bmpWidth, mImpl->bmpHeight,
        mImpl->bits, (BITMAPINFO*)&bih, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(hwnd, wdc);
}

// ============================================================================
// System
// ============================================================================

static bool g_initialized = false;

bool Initialise(const AppData&) { g_initialized = true; return true; }
void Close() { g_initialized = false; }
void PollEvents() { MSG msg; while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); } }

} // namespace nkentseu