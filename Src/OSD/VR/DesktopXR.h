#ifndef INCLUDED_DESKTOPXR_H
#define INCLUDED_DESKTOPXR_H

// OpenXR layer for desktop Supermodel (PC VR: Virtual Desktop / Quest Link / SteamVR).
//
// The OpenXR loader (openxr_loader.dll / libopenxr_loader.so.1) is loaded at run time, so
// the emulator still starts without it when VR is not requested.
//
// Frame model, every headset frame:
//   PollEvents() -> BeginFrame() -> [run 0..2 emulator frames, or New3D::RedrawVR()]
//   -> DrawHud() -> DrawEyesIfMissing() -> EndFrame()
// New3D draws its scene once per eye through IVRFrameTarget (BeginEye/EndEye).
//
// Layers: a projection layer (3D world + 2D background at infinity) and a quad layer with
// the 2D top layer (score, text, the game's own crosshair) placed exactly over the game's
// screen frustum at hudDistance metres.

#define XR_NO_PROTOTYPES
#include <openxr/openxr.h>

#include <GL/glew.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Graphics/New3D/VRHooks.h"

struct DesktopXRInput
{
  // Controller aim projected onto the game screen plane (0..1 = the screen, y down; values
  // outside 0..1 point beyond the original screen). Index 0 = right hand.
  bool  aimValid[2] = {false, false};
  bool  onScreen[2] = {false, false};
  float aimX[2]     = {0.5f, 0.5f};
  float aimY[2]     = {0.5f, 0.5f};

  bool trigger[2]    = {false, false};
  bool grip[2]       = {false, false};
  bool thumbClick[2] = {false, false};
  bool buttonA = false, buttonB = false, buttonX = false, buttonY = false;
  bool menu = false;
};

class DesktopXR final : public New3D::IVRFrameTarget
{
public:
  struct Config
  {
    float worldUnitsPerMeter = 1.0f;  // Model 3 world units per real metre (stereo scale)
    bool  positionalTracking = true;  // false = rotation + IPD only
    float hudDistance        = 20.0f; // metres from the origin to the 2D/HUD plane (far, so it
                                      // matches the depth of the 3D world: no double vision)
    float resolutionScale    = 1.0f;  // multiplier on the runtime's recommended eye size
    int   maxEyeWidth        = 1664;  // cap (Virtual Desktop can recommend 2500+ px per eye)
    float refreshHz          = 120.0f; // requested headset refresh, 0 = runtime default
    bool  threadedWait       = true;   // xrWaitFrame on its own thread (emulator never waits)
    bool  hudInEye           = true;   // draw the 2D layers into the eye images (exact registration
                                       // with the 3D) instead of a separate compositor quad layer
    int   hudLock            = 2;      // 2D layer (score, timer, text): 0 = fixed in the room,
                                       // 1 = follows the head, 2 = follows the head while the
                                       // game shows 3D (play, cutscenes), fixed for 2D menus
  };

  ~DesktopXR();

  // GL context must be current. Returns false (with LastError()) if VR is unavailable.
  bool Init(const Config& config);
  void Shutdown();
  const std::string& LastError() const { return m_error; }
  std::string RuntimeName() const { return m_runtimeName; }

  bool PollEvents();          // false = the runtime asked us to quit
  bool IsSessionRunning() const { return m_sessionRunning; }
  bool IsFocused() const { return m_focused; }   // the app is in front and gets input (headset in use)

  // xrWaitFrame + xrBeginFrame + locate views/controllers. True if eyes should be drawn.
  // EndFrame() must always follow.
  bool   BeginFrame();
  void   EndFrame();
  double FrameDeltaSeconds() const { return m_frameDelta; }
  double DisplayPeriodSeconds() const { return m_displayPeriod; }
  // Threaded wait: start waiting for the next headset frame in the background, and poll it
  // (true = the frame is ready and BeginFrame() will not block). Without the thread: always true.
  void RequestWait();
  bool WaitForFrame(int maxMicroseconds);
  // Wakes a WaitForFrame() early (e.g. the emulator finished a frame); thread-safe.
  void NotifyEvent();
  int64_t PredictedDisplayTime() const { return (int64_t)m_frameState.predictedDisplayTime; }

  void SetGameFrustum(float left, float right, float top, float bottom);
  void SetBackdropTexture(GLuint tex) { m_backdropTex = tex; }
  void SuppressEyes(bool suppress) { m_suppressEyes = suppress; }
  void DrawEyesIfMissing();

  // Draws the 2D top layer (496x384 RGBA, rows top-first; 0 = none) plus optional pointer
  // dots for each aiming hand into the HUD quad.
  // keepFirstRow/keepEndRow (0..384): outside these rows black pixels of the 2D layer are made
  // transparent - removes the game's letterbox bars in cutscenes but keeps text in them.
  // subTex/subUv: small 3D viewports rendered flat (menus), shown on the panel under the 2D layer.
  void DrawHud(GLuint topLayerTex, bool pointers, int keepFirstRow = 0, int keepEndRow = 384,
               GLuint subTex = 0, const float* subUv = nullptr);

  // Re-centre: the current head position/yaw becomes the origin facing the game screen.
  void Recenter();

  const DesktopXRInput& Input() const { return m_input; }

  // New3D::IVRFrameTarget
  int   EyeCount() const override { return 2; }
  bool  BeginEye(int eye, New3D::VREye& outEye) override;
  void  EndEye(int eye) override;
  float CullTangent() const override { return 100.f; }
  void  SetScreen(const New3D::VRScreenInfo& info) override;

  // 2D top layer texture (496x384, rows top-first) and HUD options, for the in-eye HUD.
  void SetHudSource(GLuint topLayerTex, bool pointers, bool hideLetterbox)
  {
    m_hudTopTex = topLayerTex;
    m_hudPointers = pointers;
    m_hideLetterbox = hideLetterbox;
  }

  int EyeWidth()  const { return m_eyes[0].width; }
  int EyeHeight() const { return m_eyes[0].height; }

private:
  struct Swapchain
  {
    XrSwapchain handle = XR_NULL_HANDLE;
    int width = 0, height = 0;
    std::vector<uint32_t> images;     // GL texture names
    std::vector<GLuint> fbos;
    uint32_t acquired = UINT32_MAX;
    bool releasedOnce = false;
  };

  bool Fail(const std::string& what);
  bool Check(XrResult r, const char* what);
  bool LoadLoader();
  bool CreateInstance();
  bool CreateSession();
  bool CreateSwapchain(Swapchain& sc, int w, int h);
  void DestroySwapchain(Swapchain& sc);
  bool CreateActions();
  bool CreateShaders();
  void SyncInput();
  bool AcquireImage(Swapchain& sc);
  void ReleaseImage(Swapchain& sc);
  void ComputeEye(int eye, New3D::VREye& out) const;
  void DrawBackdrop(const New3D::VREye& eye);
  void DrawHudInEye(const New3D::VREye& eye);
  void HandleSessionState(XrSessionState state);
  void RecreateLocalSpace(const XrPosef& origin);
  void StartWaitThread();
  void StopWaitThread();

  Config m_config;
  std::string m_error;
  std::string m_runtimeName;
  void* m_loaderLib = nullptr;

  XrInstance m_instance = XR_NULL_HANDLE;
  XrSystemId m_systemId = XR_NULL_SYSTEM_ID;
  XrSession  m_session  = XR_NULL_HANDLE;
  XrSpace    m_stageSpace = XR_NULL_HANDLE;   // raw LOCAL space
  XrSpace    m_localSpace = XR_NULL_HANDLE;   // LOCAL with our recentre offset
  XrSpace    m_viewSpace  = XR_NULL_HANDLE;
  std::atomic<bool> m_sessionRunning{false};
  std::atomic<bool> m_focused{false};
  bool m_exitRequested  = false;
  bool m_recenterPending = true;
  bool m_hasRefreshRate = false;

  int64_t m_colorFormat = 0;

  Swapchain m_eyes[2];
  Swapchain m_hud;
  XrView    m_views[2]{{XR_TYPE_VIEW}, {XR_TYPE_VIEW}};
  XrPosef   m_headPose{{0, 0, 0, 1}, {0, 0, 0}};
  bool      m_viewsValid = false;
  bool      m_eyeDrawn[2] = {false, false};
  bool      m_hudDrawn = false;
  bool      m_suppressEyes = false;

  XrFrameState m_frameState{XR_TYPE_FRAME_STATE};

  std::thread             m_waitThread;
  std::mutex              m_waitMutex;
  std::condition_variable m_waitCv;
  bool                    m_waitRequested = false, m_waitDone = false, m_waitQuit = false;
  bool                    m_extEvent = false;
  XrFrameState            m_waitState{XR_TYPE_FRAME_STATE};
  XrResult                m_waitResult = XR_SUCCESS;
  bool   m_frameBegun = false;
  double m_displayPeriod = 1.0 / 90.0;
  double m_frameDelta = 1.0 / 90.0;
  XrTime m_lastDisplayTime = 0;

  float m_gameL = -0.6f, m_gameR = 0.6f, m_gameT = 0.465f, m_gameB = -0.465f;

  XrActionSet m_actionSet = XR_NULL_HANDLE;
  XrAction m_aimPose = XR_NULL_HANDLE, m_trigger = XR_NULL_HANDLE, m_grip = XR_NULL_HANDLE;
  XrAction m_a = XR_NULL_HANDLE, m_b = XR_NULL_HANDLE, m_x = XR_NULL_HANDLE, m_y = XR_NULL_HANDLE;
  XrAction m_menu = XR_NULL_HANDLE, m_thumbClick = XR_NULL_HANDLE;
  XrPath   m_hands[2] = {XR_NULL_PATH, XR_NULL_PATH};  // 0 = right, 1 = left
  XrSpace  m_aimSpaces[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};
  DesktopXRInput m_input;

  GLuint m_backdropTex = 0;
  GLuint m_hudTopTex = 0;
  bool   m_hudPointers = false;
  bool   m_hideLetterbox = true;
  New3D::VRScreenInfo m_screen;
  bool   m_hudOnHead = false;   // VRHudLock 2: current choice, switched after a short steady period
  int    m_hudSwitchFrames = 0;
  New3D::VREye m_curEye[2];
  GLuint m_quadProgram = 0;     // textured quad (backdrop + HUD)
  GLint  m_uMvp = -1, m_uTex = -1, m_uRect = -1, m_uMode = -1, m_uKeep = -1, m_uSubUv = -1;
  GLuint m_dotProgram = 0;      // pointer dots on the HUD
  GLint  m_uDotCenter = -1, m_uDotRadius = -1, m_uDotColor = -1, m_uDotAspect = -1;
  GLuint m_vao = 0;
};

#endif
