#ifndef INCLUDED_VRINPUT_H
#define INCLUDED_VRINPUT_H

// Virtual mouse/keyboard fed by the VR controllers.
//
// Light gun games are driven exactly the way desktop Supermodel drives them with a mouse
// (InputAnalogGunX = MOUSE_XAXIS, trigger = MOUSE_LEFT_BUTTON, ...): the controller's aim
// ray is turned into a mouse position inside the game area of the window, and the
// triggers into mouse buttons. The SDL input system reads these instead of the real
// mouse while 'active' is set. Written and read on the main thread only.

struct VRVirtualInput
{
  bool  active = false;
  float x = 0.5f;           // 0..1 across the game screen, left -> right
  float y = 0.5f;           // 0..1 down the game screen, top -> bottom
  bool  mouseLeft = false;  // primary trigger
  bool  mouseRight = false; // other hand's trigger
  bool  mouseMiddle = false;

  // Keyboard keys (default Supermodel bindings)
  bool  start1 = false;     // KEY_1
  bool  coin1 = false;      // KEY_3
  bool  test = false;       // KEY_9 (Test A)
  bool  service = false;    // KEY_0 (Service A)
};

extern VRVirtualInput g_vrInput;

#endif
