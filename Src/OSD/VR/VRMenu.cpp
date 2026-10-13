#include "VRMenu.h"

#include "Pkgs/imgui/imgui.h"
#include "Pkgs/imgui/imgui_impl_opengl3.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>

namespace
{
  // A row of buttons, the current one highlighted - easier to hit with a laser than a combo box.
  bool Choice(const char *label, int *value, const char *const *names, int count)
  {
    bool changed = false;
    ImGui::TextUnformatted(label);
    ImGui::PushID(label);
    const float avail = ImGui::GetContentRegionAvail().x;
    const float w = (avail - ImGui::GetStyle().ItemSpacing.x * (count - 1)) / (float)count;
    for (int i = 0; i < count; i++)
    {
      if (i)
        ImGui::SameLine();
      const bool on = (*value == i);
      if (on)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.45f, 0.10f, 1.0f));
      if (ImGui::Button(names[i], ImVec2(w, 0)) && !on)
      {
        *value = i;
        changed = true;
      }
      if (on)
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
    return changed;
  }

  bool Toggle(const char *label, bool *value)
  {
    static const char *const kOnOff[] = { "Off", "On" };
    int v = *value ? 1 : 0;
    if (!Choice(label, &v, kOnOff, 2))
      return false;
    *value = (v != 0);
    return true;
  }

  // - value + buttons, so it can be set precisely with a pointer
  bool Stepper(const char *label, float *value, float step, float lo, float hi, const char *fmt)
  {
    bool changed = false;
    ImGui::TextUnformatted(label);
    ImGui::PushID(label);
    const float bw = 110.0f;
    if (ImGui::Button("-", ImVec2(bw, 0)))
    {
      *value = std::max(lo, *value - step);
      changed = true;
    }
    ImGui::SameLine();
    char text[32];
    std::snprintf(text, sizeof(text), fmt, *value);
    const float tw = ImGui::GetContentRegionAvail().x - bw - ImGui::GetStyle().ItemSpacing.x;
    ImGui::Button(text, ImVec2(tw, 0));
    ImGui::SameLine();
    if (ImGui::Button("+", ImVec2(bw, 0)))
    {
      *value = std::min(hi, *value + step);
      changed = true;
    }
    ImGui::PopID();
    if (changed)   // keep it on the step grid (no 0.30000001)
      *value = (float)((int)(*value / step + (*value >= 0 ? 0.5f : -0.5f))) * step;
    return changed;
  }
}

VRMenu::~VRMenu()
{
  Shutdown();
}

bool VRMenu::Init()
{
  if (m_ready)
    return true;

  ImGuiContext *previous = ImGui::GetCurrentContext();
  ImGuiContext *ctx = ImGui::CreateContext();
  m_ctx = ctx;
  ImGui::SetCurrentContext(ctx);
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr;   // no imgui.ini next to the game
  io.LogFilename = nullptr;
  io.DisplaySize = ImVec2((float)Width, (float)Height);
  ImFontConfig font;
  font.SizePixels = 30.0f;
  font.OversampleH = font.OversampleV = 1;
  io.Fonts->AddFontDefault(&font);

  ImGuiStyle &style = ImGui::GetStyle();
  ImGui::StyleColorsDark(&style);
  style.WindowPadding = ImVec2(28, 22);
  style.FramePadding = ImVec2(12, 12);
  style.ItemSpacing = ImVec2(12, 10);
  style.FrameRounding = 8.0f;
  style.WindowRounding = 0.0f;
  style.WindowBorderSize = 0.0f;
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.96f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.32f, 0.36f, 0.44f, 1.0f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.95f, 0.55f, 0.15f, 1.0f);
  style.Colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.17f, 0.21f, 1.0f);
  style.Colors[ImGuiCol_TabHovered] = ImVec4(0.32f, 0.36f, 0.44f, 1.0f);
  style.Colors[ImGuiCol_TabActive] = ImVec4(0.85f, 0.45f, 0.10f, 1.0f);

  const bool ok = ImGui_ImplOpenGL3_Init("#version 330");

  glGenTextures(1, &m_tex);
  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
  const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ImGui::SetCurrentContext(previous);
  m_ready = ok && complete;
  if (!m_ready)
    Shutdown();
  return m_ready;
}

void VRMenu::Shutdown()
{
  if (m_ctx)
  {
    ImGuiContext *previous = ImGui::GetCurrentContext();
    ImGui::SetCurrentContext((ImGuiContext *)m_ctx);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext((ImGuiContext *)m_ctx);
    ImGui::SetCurrentContext(previous == m_ctx ? nullptr : previous);
    m_ctx = nullptr;
  }
  if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
  if (m_tex) glDeleteTextures(1, &m_tex);
  m_fbo = m_tex = 0;
  m_ready = false;
}

VRMenuResult VRMenu::Render(VRSettings &s, float pointerU, float pointerV, bool valid, bool press, double dt)
{
  VRMenuResult r;
  if (!m_ready)
    return r;

  ImGuiContext *previous = ImGui::GetCurrentContext();
  ImGui::SetCurrentContext((ImGuiContext *)m_ctx);
  ImGuiIO &io = ImGui::GetIO();
  io.DeltaTime = (float)std::clamp(dt, 1.0 / 240.0, 0.25);
  io.DisplaySize = ImVec2((float)Width, (float)Height);
  if (valid)
    io.AddMousePosEvent(pointerU * Width, pointerV * Height);
  else
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  io.AddMouseButtonEvent(0, valid && press);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2((float)Width, (float)Height));
  ImGui::Begin("L.A. Machineguns VR", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
               ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);

  ImGui::TextUnformatted("L.A. MACHINEGUNS VR - OPTIONS");
  ImGui::Separator();

  if (ImGui::BeginTabBar("tabs"))
  {
    if (ImGui::BeginTabItem("  Comfort  "))
    {
      static const char *const kVignette[] = { "Off", "Light", "Medium", "Strong" };
      r.changed |= Choice("Vignette during fast camera turns", &s.vignette, kVignette, 4);
      ImGui::Spacing();
      r.changed |= Toggle("Cinema mode (game on a big screen, no motion)", &s.cinema);
      static const char *const kSize[] = { "Small", "Medium", "Large" };
      ImGui::BeginDisabled(!s.cinema);
      r.changed |= Choice("Cinema screen size", &s.cinemaSize, kSize, 3);
      static const char *const kDepth[] = { "Flat", "Shallow", "Full" };
      r.changed |= Choice("Cinema 3D depth", &s.cinemaDepth, kDepth, 3);
      ImGui::EndDisabled();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("  Gun  "))
    {
      static const char *const kHand[] = { "Either (last fired)", "Right", "Left" };
      r.changed |= Choice("Gun hand", &s.gunHand, kHand, 3);
      static const char *const kRecoil[] = { "Off", "Light", "Strong" };
      r.changed |= Choice("Recoil vibration", &s.recoil, kRecoil, 3);
      r.changed |= Stepper("Aim fine-tune: left / right", &s.aimNudgeX, 0.1f, -3.0f, 3.0f, "%+.1f deg");
      r.changed |= Stepper("Aim fine-tune: down / up", &s.aimNudgeY, 0.1f, -3.0f, 3.0f, "%+.1f deg");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("  Screen  "))
    {
      static const char *const kLock[] = { "Fixed", "Follow head", "Auto" };
      int lock = s.hudLock == 0 ? 0 : (s.hudLock == 1 ? 1 : 2);
      if (Choice("Score / timer / text", &lock, kLock, 3))
      {
        s.hudLock = lock;
        r.changed = true;
      }
      static const char *const kFilter[] = { "Soft", "Sharp", "Smooth edges" };
      r.changed |= Choice("2D graphics scaling", &s.hudFilter, kFilter, 3);
      r.changed |= Toggle("Hide player 2 PRESS START during play", &s.hideP2);
      r.changed |= Toggle("Hit static fills the whole view", &s.staticFill);
      r.changed |= Stepper("Render resolution (next start)", &s.renderScale, 0.1f, 0.5f, 1.5f, "%.1fx");
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }

  // Bottom row
  const float rowY = (float)Height - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().WindowPadding.y;
  ImGui::SetCursorPosY(rowY);
  const float bw = (ImGui::GetContentRegionAvail().x - 2 * ImGui::GetStyle().ItemSpacing.x) / 3.0f;
  if (ImGui::Button("Resume", ImVec2(bw, 0)))
    r.resume = true;
  ImGui::SameLine();
  if (ImGui::Button("Re-centre view", ImVec2(bw, 0)))
    r.recenter = true;
  ImGui::SameLine();
  if (ImGui::Button("Quit game", ImVec2(bw, 0)))
    r.quit = true;

  ImGui::End();
  ImGui::Render();

  GLint prevFbo = 0, viewport[4];
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
  glGetIntegerv(GL_VIEWPORT, viewport);
  const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(0, 0, Width, Height);
  glDisable(GL_SCISSOR_TEST);
  glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFbo);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
  if (scissor)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  ImGui::SetCurrentContext(previous);
  return r;
}
