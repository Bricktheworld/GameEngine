#pragma once
#include <editor/imgui/window.hpp>

namespace Brick3D::Editor
{
class SceneWindow : public Window
{
  public:
    void Render() override;
};
}; // namespace Brick3D::Editor