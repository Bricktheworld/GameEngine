#pragma once
#include <editor.h>
#include <ImGuizmo/ImGuizmo.h>

struct SceneWindow
{
    ImGuizmo::OPERATION current_operation = ImGuizmo::OPERATION::TRANSLATE;
};

void register_scene_window(Vultr::Engine *e);
void scene_window_render(Vultr::Engine *e, const Vultr::UpdateTick &tick, void *state);
