Windows build of Supermodel with the OpenXR VR mode, set up for **L.A. Machineguns**.

**Download** `SupermodelVR-windows.zip` below and unzip it. Put your own `lamachin.zip` ROM set in the `ROMs` folder. Connect the headset, with Virtual Desktop, Quest Link or SteamVR as the active OpenXR runtime, and run `Play LA Machineguns VR.bat`. Full instructions are in `README-VR.txt` inside the zip and in the README. No ROMs or other game files are included.

## What's new since v0.1.0

### Two guns at once (akimbo)
- New **Gun hand › Both** option: a gun in each hand for one player (`VRGunHand = 3`).
- With both triggers held, the game's single gun takes turns between your two aim points every game frame, so each hand gets about half the shots. The game's crosshair flickers between both points, and both controllers vibrate.
- With one trigger held, that hand works as a normal gun.

### In-headset options menu
Press the **left menu button** to open it; the game pauses while it's open. Point with a controller and pull the trigger to choose. Every change is saved to `Config/Supermodel.ini` straight away.
- **Comfort**
  - **Vignette:** darkens the edges of your view during fast camera turns (Off / Light / Medium / Strong). It helps against motion sickness.
  - **Cinema mode:** plays the game on a big screen in a dark room, so the camera never moves around you. Small, medium or large screen.
  - **3D depth in cinema mode:** Flat / Shallow / Full. With depth on, the game's world sits behind the screen like a 3D TV.
- **Gun**
  - **Gun hand:** Either / Right / Left / Both.
  - **Recoil vibration:** Off / Light / Strong.
  - **Aim fine-tune:** nudges the aim left/right and up/down in 0.1° steps.
- **Screen**
  - **Score and text:** fixed in the room, following your head, or Auto (follows your head during play).
  - **2D graphics scaling:** Soft / Sharp / Smooth edges.
  - **Hide player 2's prompt** and **hit static over the whole view** (see below).
  - **Render resolution:** 0.5× to 1.5×, applied on the next start.
- Buttons for **Resume**, **Re-centre** and **Quit game**.
- The menu uses the Michroma typeface (SIL Open Font License). You can frame it with your own artwork: see `Assets/VR-MENU.txt`.

### Cleaner screen during play
- **Player 2's "PRESS START BUTTON" is hidden** during single-player play, including while the screen shakes when you're hit. Only the prompt itself is removed, so the Japanese subtitles and everything else in that corner stay. The title and continue screens keep their prompts.
- **Hit static fills your whole view.** When you take damage, the game's flickering static now covers everything you see, not just the original 4:3 screen.

### Controls
- Either stick click re-centres the view. Left grip + left stick click still opens the operator (test) menu.
