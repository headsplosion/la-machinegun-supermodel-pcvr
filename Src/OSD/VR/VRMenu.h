#ifndef INCLUDED_VRMENU_H
#define INCLUDED_VRMENU_H

// In-headset options menu (PC VR). Drawn with Dear ImGui into an off-screen texture that the
// VR layer shows as a panel in front of the player; the controller is the mouse.

#include <GL/glew.h>

// Every option the menu can change. Saved to Config/Supermodel.ini when changed.
struct VRSettings
{
  int   gunHand     = 0;      // 0 = whichever hand fires, 1 = right, 2 = left
  int   vignette    = 2;      // comfort vignette during fast camera turns: 0 off .. 3 strong
  bool  cinema      = false;  // play on a big screen in a dark room
  int   cinemaSize  = 1;      // 0 small, 1 medium, 2 large
  int   cinemaDepth = 2;      // 3D depth in cinema mode: 0 off, 1 shallow, 2 full
  int   hudLock     = 2;      // 2D layer: 0 room, 1 head, 2 head during play
  bool  hideP2      = true;   // hide player 2's PRESS START BUTTON during play
  bool  staticFill  = true;   // hit static over the whole view
  int   recoil      = 1;      // controller vibration while firing: 0 off, 1 light, 2 strong
  float aimNudgeX   = 0.0f;   // aim fine-tune, degrees (+ = right)
  float aimNudgeY   = 0.0f;   // aim fine-tune, degrees (+ = up)
  float renderScale = 1.0f;   // eye resolution, applied on the next start
  int   hudFilter   = 1;      // 2D layer scaling: 0 soft, 1 sharp, 2 smooth edges
};

struct VRMenuResult
{
  bool changed  = false;   // a setting changed (save it)
  bool resume   = false;
  bool recenter = false;
  bool quit     = false;
};

class VRMenu
{
public:
  static constexpr int Width  = 960;
  static constexpr int Height = 720;

  ~VRMenu();
  bool   Init();                 // GL context must be current
  void   Shutdown();
  // Draws the menu into Texture(). pointer: position on the panel (0..1, y down), valid = the
  // controller points at it; press = trigger held.
  VRMenuResult Render(VRSettings &s, float pointerU, float pointerV, bool valid, bool press, double dt);
  GLuint Texture() const { return m_tex; }

private:
  void *m_ctx = nullptr;         // ImGuiContext
  GLuint m_tex = 0, m_fbo = 0;
  bool m_ready = false;
};

#endif
