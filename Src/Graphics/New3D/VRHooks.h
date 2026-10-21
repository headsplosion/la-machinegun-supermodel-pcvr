#ifndef _NEW3D_VRHOOKS_H_
#define _NEW3D_VRHOOKS_H_

// Minimal bridge between the New3D renderer and a stereo (OpenXR) frame target.
//
// New3D stays platform-agnostic: it only knows that, when a target is installed,
// the scene it built for the frame must be drawn once per eye, with
//   - an eye-specific asymmetric projection (tangents of the eye FOV), and
//   - an eye-specific view matrix pre-multiplied onto every model matrix.
//
// Model 3 bakes the game camera into each model matrix (InitMatrixStack loads the
// coordinate-system matrix, then the game's camera/object matrices). The result is
// OpenGL view space: +X right, +Y up, -Z forward -- the same convention OpenXR uses.
// So head tracking is simply: modelMat' = eyeView * modelMat.

#include <vector>

namespace New3D {

struct VREye
{
	// Tangents of the eye FOV half-angles (OpenXR: tan(angleLeft) < 0, tan(angleRight) > 0,
	// tan(angleUp) > 0, tan(angleDown) < 0).
	float tanLeft	= -1.f;
	float tanRight	=  1.f;
	float tanUp		=  1.f;
	float tanDown	= -1.f;

	bool  stereo		= false;		// flat target drawn as one view of a stereo pair (3D cinema)
	float convergence	= 0.f;			// 3D cinema: depth that sits on the screen surface

	// Column-major 4x4: inverse of the eye pose relative to the recentred origin,
	// translation already converted from metres to Model 3 world units.
	float view[16]	= { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	// Framebuffer to composite this eye's 3D into (desktop path); 0 = currently bound.
	unsigned fbo	= 0;
	int width		= 0;
	int height		= 0;
};

// What the renderer knows about this frame's screen, handed to the frame target before the eyes
// are drawn so it can place the 2D layers exactly over the 3D view.
struct VRScreenInfo
{
	bool		hasFrustum = false;					// false: keep the previous frustum
	int			mainModels = 0;						// models in the main 3D view (0 = no 3D on screen)
	float		left = 0, right = 0, top = 0, bottom = 0;	// tangents of the full 496x384 screen
	int			viewFirstRow = 0, viewEndRow = 384;	// rows covered by the main 3D view (letterbox)
	unsigned	subTexture = 0;						// small viewports drawn flat (0 = none)
	float		subUv[4] = { 0, 0, 1, 1 };			// game area in that texture (u0, v0, u1, v1)
	float		cameraTurn = 0;						// game camera rotation since the last game frame (degrees)
};

// A model the VR layer wants held in the player's hand (L.A. Machineguns: the S.I.F.A.T.
// officers' gun). pose: column-major controller pose (rotation, translation already in world
// units) relative to the recentred origin; unitsPerMeter: world units per metre.
struct VRHandPose
{
	float pose[16];
	float unitsPerMeter;
};

// Which ROM model to hold, and how. Set by the platform layer; 0 = none.
struct VRHandModel
{
	unsigned	address		= 0;		// Real3D VROM model address
	float		length		= 0.75f;	// metres, along the model's longest side
	float		gripFrac	= 0.18f;	// where the hand holds it: fraction of the length from the bulky end
	bool		flip		= false;	// barrel pointing the wrong way: turn it round
	bool		upsideDown	= false;	// roll it over
};
extern VRHandModel g_vrHandModel;

// The same hand model as plain triangles in "hand space" (metres; barrel along -Z, +Y up, grip at
// the origin), for the VR layer to draw it itself (cinema mode). 9 floats per vertex: position,
// normal, colour. 'version' changes whenever it is rebuilt.
struct VRHandMesh
{
	std::vector<float>	verts;
	unsigned			version = 0;
};
extern VRHandMesh g_vrHandMesh;

class IVRFrameTarget
{
public:
	virtual ~IVRFrameTarget() = default;

	virtual int  EyeCount() const = 0;

	// Bind the eye's render target, set viewport, clear, draw any background layer.
	// Fills 'eye' with this eye's projection tangents and view matrix.
	// Returns false to skip the eye.
	virtual bool BeginEye(int eye, VREye& outEye) = 0;
	virtual void EndEye(int eye) = 0;

	// Called once per drawn game frame, before the eyes.
	virtual void SetScreen(const VRScreenInfo& info) { (void)info; }

	// Cinema mode: return true and a render target to get the game drawn flat (as on a monitor)
	// into it instead of once per eye; the eyes are then composed by the target itself.
	// view: 0..FlatViewCount()-1. With 2 views (3D cinema), outTarget also carries the view's
	// off-axis frustum and eye offset (stereo = true), and convergence = depth (world units)
	// that lands on the screen surface.
	virtual int  FlatViewCount() const { return 1; }
	virtual bool BeginFlat(int view, VREye& outTarget) { (void)view; (void)outTarget; return false; }
	virtual void EndFlat(int view) { (void)view; }
	virtual bool WantsFlat() const { return false; }

	// Half-angle tangent used to widen renderer-side culling while VR is active.
	virtual float CullTangent() const { return 2.5f; }

	// Controller poses that should hold g_vrHandModel this headset frame (up to 2).
	virtual int  HandPoses(VRHandPose* out, int max) const { (void)out; (void)max; return 0; }
};



// Global hook (set by the platform layer, read by New3D). nullptr = flat rendering.
void			SetVRFrameTarget(IVRFrameTarget* target);
IVRFrameTarget*	GetVRFrameTarget();

// The game's main 3D view: a viewport spanning the whole screen width. Cutscenes use a
// letterboxed one (e.g. 496x280 at y 52), which must get stereo + head tracking too.
inline bool VRIsMainViewport(int vpX, int vpWidth, int vpHeight)
{
	return vpX == 0 && vpWidth >= 495 && vpHeight >= 96;
}

// Column-major 4x4 multiply: out = a * b. 'out' may alias neither input.
inline void VRMul4(const float a[16], const float b[16], float out[16])
{
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			out[c * 4 + r] =
				a[0 * 4 + r] * b[c * 4 + 0] +
				a[1 * 4 + r] * b[c * 4 + 1] +
				a[2 * 4 + r] * b[c * 4 + 2] +
				a[3 * 4 + r] * b[c * 4 + 3];
		}
	}
}

// Rotate a direction by the upper 3x3 of a column-major 4x4.
inline void VRRotate3(const float m[16], const float in[3], float out[3])
{
	out[0] = m[0] * in[0] + m[4] * in[1] + m[8]  * in[2];
	out[1] = m[1] * in[0] + m[5] * in[1] + m[9]  * in[2];
	out[2] = m[2] * in[0] + m[6] * in[1] + m[10] * in[2];
}

} // New3D

#endif
