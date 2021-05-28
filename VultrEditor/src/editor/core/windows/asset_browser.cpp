#include <editor/core/windows/asset_browser.hpp>
#include <editor/editor.hpp>
#include <imgui/imgui.h>
#include <types/types.hpp>
#include <fonts/fork_awesome.h>

using namespace Vultr;
static Texture *get_texture_from_file(const std::string &name)
{
    std::string t = Path::get_resource_type(name);
    auto *e = Editor::Get();
    if (t == SHADER_FILE)
    {
        return e->shader_icon;
    }
    else if (t == HEADER_FILE)
    {
        return e->c_icon;
    }
    else if (t == SOURCE_FILE)
    {
        return e->c_icon;
    }
    else if (t == IMAGE_FILE)
    {
        return e->image_icon;
    }
    else if (t == MODEL_FILE)
    {
        return e->model_icon;
    }
    else
    {
        return e->file_icon;
    }
}

void AssetBrowser::Render()
{
    auto &current_directory = Editor::Get()->current_directory;
    static std::vector<File> files;
    static std::vector<Directory> sub_directories;
    static s32 selected = -1;
    if (files.empty() && sub_directories.empty())
    {
        files = current_directory.Files();
        sub_directories = current_directory.Directories();
    }
    ImGui::Begin("Asset Browser");
    s32 num_cols = ImGui::GetWindowWidth() / 200 - 2;
    if (num_cols < 1)
        num_cols = 1;
    ImGui::SetCursorPosY(ImGui::GetCursorPosX() + 150);
    if (ImGui::BeginTable("table1", num_cols))
    {
        s32 index = 0;

        while (index < files.size() + sub_directories.size())
        {
            ImGui::TableNextRow();
            for (int column = 0; column < num_cols; column++)
            {
                if (index >= files.size() + sub_directories.size())
                    break;
                std::string label;
                bool is_directory = index < sub_directories.size();
                if (!is_directory)
                {
                    label = files[index - sub_directories.size()].GetName();
                }
                else
                {
                    label = sub_directories[index].GetName();
                }
                ImGui::TableSetColumnIndex(column);
                ImGui::PushID(index);
                if (ImGui::Selectable("##", selected == index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(200, 200)))
                {
                    selected = index;
                }
                if (ImGui::BeginDragDropSource())
                {
                    void *payload = is_directory ? (void *)&sub_directories[index] : (void *)&files[index - sub_directories.size()];
                    ImGui::SetDragDropPayload(is_directory ? "Directory" : "File", payload, sizeof(Vultr::File));
                    auto *texture = is_directory ? Editor::Get()->folder_icon : get_texture_from_file(files[index - sub_directories.size()].GetName());
                    texture->Bind(GL_TEXTURE0);
                    ImGui::Image((void *)(intptr_t)texture->GetID(), ImVec2(125, 125));
                    ImGui::Text("%s", label.c_str());
                    ImGui::EndDragDropSource();
                }
                if (is_directory && ImGui::BeginDragDropTarget())
                {
                    const auto *payload = ImGui::AcceptDragDropPayload("File");
                    ImGui::EndDragDropTarget();
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    if (index < sub_directories.size())
                    {
                        current_directory = sub_directories[index];
                        files = current_directory.Files();
                        sub_directories = current_directory.Directories();
                        selected = -1;
                        ImGui::PopID();
                        ImGui::EndTable();
                        ImGui::End();
                        return;
                    }
                }
                auto *texture = is_directory ? Editor::Get()->folder_icon : get_texture_from_file(files[index - sub_directories.size()].GetName());
                texture->Bind(GL_TEXTURE0);
                ImGui::SameLine((200.0 - 125.0) / 2.0);
                ImGui::Image((void *)(intptr_t)texture->GetID(), ImVec2(125, 125));
                double inset = 160 - ImGui::GetFontSize() + ImGui::GetFontSize() / 2;
                ImGui::SameLine(20);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 125);
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 180);
                ImGui::Text("%s", label.c_str());
                ImGui::PopTextWrapPos();

                ImGui::PopID();
                index++;
            }
        }
        ImGui::EndTable();
    }
    ImGui::SameLine(10);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 100);
    ImGui::Text("%s", current_directory.GetPath().c_str());
    ImGui::SameLine(10);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50);

    static bool actioned_last_frame = false;
    int back_button_state = glfwGetMouseButton(engine_global()->window, GLFW_MOUSE_BUTTON_4);
    if (ImGui::Button(ICON_FK_BACKWARD) || (back_button_state == GLFW_PRESS && !actioned_last_frame))
    {
        actioned_last_frame = true;
        if (current_directory != Path::get_resource_path())
        {
            auto path = current_directory.GetPath();
            if (path.has_parent_path())
            {
                current_directory = Directory(path.parent_path());
                files = current_directory.Files();
                sub_directories = current_directory.Directories();
                selected = -1;
            }
        }
    }
    else if (back_button_state == GLFW_RELEASE)
    {
        actioned_last_frame = false;
    }

    ImGui::End();
}
