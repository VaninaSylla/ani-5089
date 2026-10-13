// win32_renderer.h — Rendu software Win32 minimal pour les exercices

#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <memory>

namespace nkentseu {

// ---------------------------------------------------------------------------
// Types mathématiques (compatibles NKMath)
// ---------------------------------------------------------------------------

struct Vec2f { float x, y; };
struct Vec3f { float x, y, z; };

struct Mat4f {
    float m[4][4];
    Mat4f() { for(int i=0;i<4;++i)for(int j=0;j<4;++j)m[i][j]=(i==j)?1:0; }
    
    static Mat4f Translate(const Vec3f& v) {
        Mat4f r; r.m[0][3]=v.x; r.m[1][3]=v.y; r.m[2][3]=v.z; return r;
    }
    static Mat4f Scale(const Vec3f& v) {
        Mat4f r; r.m[0][0]=v.x; r.m[1][1]=v.y; r.m[2][2]=v.z; return r;
    }
    static Mat4f Perspective(float fovY, float aspect, float nearZ, float farZ) {
        Mat4f r; float tanHalfFov = tanf(fovY * 0.5f);
        r.m[0][0]=1/(aspect*tanHalfFov); r.m[1][1]=1/tanHalfFov;
        r.m[2][2]=-(farZ+nearZ)/(farZ-nearZ); r.m[2][3]=-(2*farZ*nearZ)/(farZ-nearZ);
        r.m[3][2]=-1; r.m[3][3]=0; return r;
    }
    static Mat4f LookAt(const Vec3f& eye, const Vec3f& target, const Vec3f& up) {
        Mat4f r;
        Vec3f z = {eye.x-target.x, eye.y-target.y, eye.z-target.z};
        float zl = sqrtf(z.x*z.x+z.y*z.y+z.z*z.z); if(zl>0){z.x/=zl;z.y/=zl;z.z/=zl;}
        Vec3f x = {up.y*z.z-up.z*z.y, up.z*z.x-up.x*z.z, up.x*z.y-up.y*z.x};
        float xl = sqrtf(x.x*x.x+x.y*x.y+x.z*x.z); if(xl>0){x.x/=xl;x.y/=xl;x.z/=xl;}
        Vec3f y = {z.y*x.z-z.z*x.y, z.z*x.x-z.x*x.z, z.x*x.y-z.y*x.x};
        r.m[0][0]=x.x; r.m[0][1]=x.y; r.m[0][2]=x.z; r.m[0][3]=-(x.x*eye.x+x.y*eye.y+x.z*eye.z);
        r.m[1][0]=y.x; r.m[1][1]=y.y; r.m[1][2]=y.z; r.m[1][3]=-(y.x*eye.x+y.y*eye.y+y.z*eye.z);
        r.m[2][0]=z.x; r.m[2][1]=z.y; r.m[2][2]=z.z; r.m[2][3]=-(z.x*eye.x+z.y*eye.y+z.z*eye.z);
        return r;
    }
    Mat4f operator*(const Mat4f& o) const {
        Mat4f r; for(int i=0;i<4;++i)for(int j=0;j<4;++j){r.m[i][j]=0;for(int k=0;k<4;++k)r.m[i][j]+=m[i][k]*o.m[k][j];} return r;
    }
    Vec3f operator*(const Vec3f& v) const {
        return {m[0][0]*v.x+m[0][1]*v.y+m[0][2]*v.z+m[0][3],
                m[1][0]*v.x+m[1][1]*v.y+m[1][2]*v.z+m[1][3],
                m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z+m[2][3]};
    }
};

struct AABB { Vec3f min, max; bool IsValid() const { return min.x<=max.x && min.y<=max.y && min.z<=max.z; } };

// ---------------------------------------------------------------------------
// API Renderer 3D (compatibilité exercices)
// ---------------------------------------------------------------------------

enum RendererSubsystem { SS_NONE=0, SS_RENDER2D=1, SS_RENDER3D=2, SS_SHADOW=4, SS_RAYTRACE=8, SS_VISIBILITY=16, SS_ALL=0xFFFFFFFF };
inline RendererSubsystem operator|(RendererSubsystem a, RendererSubsystem b){ return RendererSubsystem(int(a)|int(b)); }
inline RendererSubsystem operator&(RendererSubsystem a, RendererSubsystem b){ return RendererSubsystem(int(a)&int(b)); }

enum class LightType { DIRECTIONAL=0, POINT=1, SPOT=2 };

struct Light {
    LightType type = LightType::DIRECTIONAL;
    Vec3f direction = {0,-1,0}, position = {0,0,0}, color = {1,1,1};
    float intensity = 1, range = 100, innerAngle = 0, outerAngle = 0;
    bool castShadow = false;
};

struct SceneContext {
    Mat4f viewMatrix, projMatrix;
    Vec3f cameraPos;
    std::vector<Light> lights;
};

struct DrawCall3D {
    uint32_t meshHandle = 0;
    Mat4f transform;
    Vec3f tint = {1,1,1};
    AABB aabb;
    uint32_t materialIndex = 0;
};

struct MeshSystem {
    static uint32_t GetCube() { return 1; }
    static uint32_t GetPlane() { return 2; }
};

struct Renderer3DConfig {
    RendererSubsystem subsystems = SS_RENDER3D;
    bool vsync = true, debug = false;
};

struct Stats { uint32_t drawCalls = 0, triangles = 0; };

enum class GraphicsApi { NONE=0, SOFTWARE, OPENGL, VULKAN, DIRECTX11, DIRECTX12, METAL };

// Forward declarations
struct Window;

// ---------------------------------------------------------------------------
// Window wrapper
// ---------------------------------------------------------------------------

struct WindowConfig {
    const char* title = "Window";
    uint32_t width = 1280, height = 720;
    bool resizable = true;
};

class Window {
public:
    Window();
    explicit Window(const WindowConfig& config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

    bool Create(const WindowConfig& config);
    void Close();
    bool IsOpen() const;
    bool IsValid() const;
    void SetVisible(bool visible);
    void SetTitle(const char* title);
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    HWND GetHandle() const;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

// ---------------------------------------------------------------------------
// Renderer3D
// ---------------------------------------------------------------------------

class Renderer3D {
public:
    Renderer3D();
    Renderer3D(Window& window, const Renderer3DConfig& config = {});
    ~Renderer3D();

    Renderer3D(const Renderer3D&) = delete;
    Renderer3D& operator=(const Renderer3D&) = delete;
    Renderer3D(Renderer3D&&) = default;
    Renderer3D& operator=(Renderer3D&&) = default;

    bool Create(Window& window, const Renderer3DConfig& config = {});
    void Shutdown();
    bool IsValid() const;

    GraphicsApi GetApi() const;
    const char* GetApiName() const;

    void BeginFrame(uint32_t clearColor = 0xFF202030);
    void SetScene(const SceneContext& scene);
    void Submit(const DrawCall3D& drawCall);
    void Execute();
    void Present();

    Stats GetStats() const { return mStats; }

public:
    struct Impl {
        Window* window = nullptr;
        Renderer3DConfig config;
        SceneContext scene;
        std::vector<DrawCall3D> drawCalls;
        HDC hdc = nullptr;
        HBITMAP hbmp = nullptr;
        void* bits = nullptr;
        int bmpWidth = 0, bmpHeight = 0;
        uint32_t clearColor = 0xFF202030;
    };
private:
    std::unique_ptr<Impl> mImpl;
    Stats mStats;
};

// ---------------------------------------------------------------------------
// System init
// ---------------------------------------------------------------------------

struct AppData {
    bool enableRendererDebug = false;
    bool enableEventLogging = false;
    const char* appName = "App";
    const char* appVersion = "1.0";
    bool enableMultiWindow = true;
    void* userData = nullptr;
};

bool Initialise(const AppData& data = {});
void Close();
void PollEvents();

} // namespace nkentseu