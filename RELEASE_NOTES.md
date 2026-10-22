Windows build of Supermodel with the OpenXR VR mode, set up for **L.A. Machineguns**.

**Download** `SupermodelVR-windows.zip` below and unzip it. Put your own `lamachin.zip` ROM set in the `ROMs` folder. Connect the headset, with Virtual Desktop, Quest Link or SteamVR as the active OpenXR runtime, and run `Play LA Machineguns VR.bat`. Full instructions are in `README-VR.txt` inside the zip and in the README. No ROMs or other game files are included.

## What's new in v0.5.0

### A gun in your hand
- New option: **Gun › Gun model in your hand** (Off / Game / Custom; `VRGunModel`, Game by default).
- **Game:** the gun the S.I.F.A.T. officers hold in the game's cutscenes, in the hand that holds the gun (one in each hand in akimbo mode). It is read from your own ROM while the game runs, so no game data is included.
- **Custom:** bring your own 3D model as `Assets/VRGunModel.obj` (see the README for the format).
- Shown from the moment the game starts, in cinema mode too (in front of the screen, lit by the picture), and always in front of the options menu when it's nearer to you.
- Matte plastic finish with soft reflections of the room and the screen; your own model's sheen can be tuned in its `.mtl` file.

### TV-cabinet bezel in cinema mode
- New option: **Comfort › Screen bezel** (Off / On; `VRCinemaBezel`, On by default).
- The cinema screen sits in a recessed, TV-cabinet style frame. Its inner walls show a soft, hazy reflection of the picture, like matte plastic, and the front of the frame is lit by the screen. Works together with Screen glow.

### Move the options menu
- Point at the menu and **hold the grip (side) button** to pick it up; it follows your hand and turns to face you. Let go to drop it. A short vibration marks pick-up and drop.

### Also included
Everything from v0.4.0 is still here: screen glow, the bold and sharper options menu, two-gun akimbo mode, cinema mode with 3D depth, the comfort vignette, the hidden player 2 prompt and full-view hit static. See the README for all features, controls and options.
