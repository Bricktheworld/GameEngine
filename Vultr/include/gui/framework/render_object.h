#pragma once
#include <gui/rendering/gui_vertex.h>
#include "build_context.h"
#include "box_constraints.h"

namespace Vultr
{
namespace GUI
{
class Widget;

class RenderObject
{
  public:
    RenderObject(Widget *widget) : configuration(widget), position(Vec2(0, 0))
    {
    }

    virtual void DeleteRenderObject(BuildContext *context)
    {
        if (quad_id != -1)
        {
            context->DeleteQuad(quad_id);
        }
    }

    virtual ~RenderObject()
    {
    }

    virtual void Paint(BuildContext *context) = 0;

    virtual Size Layout(BuildContext *context, BoxConstraints constraints) = 0;

    // This is only used so that if you want to lay out a flex widget
    virtual Size GetSize(BuildContext *context, BoxConstraints constraints)
    {
        return Layout(context, constraints);
    }

    void MarkForRepaint()
    {
        repaint_required = true;
    }

    bool NeedsRepaint(BuildContext *context)
    {
        if (repaint_required)
            return true;
        if (position != context->GetPosition())
        {
            position = context->GetPosition();
            return true;
        }
        return false;
    }

    virtual void Reattach(Widget *new_config)
    {
        this->configuration = new_config;
        MarkForRepaint();
    }

  protected:
    bool repaint_required = true;
    QuadID quad_id = -1;
    Widget *configuration;
    QuadProperties properties;

    virtual Widget *GetConfig()
    {
        return configuration;
    }

    Quad GetQuad(BuildContext *context, int index)
    {
        return context->GetQuad(index);
    }

    Size GetSize()
    {
        return size;
    }

    Vec2 GetPosition()
    {
        return position;
    }

    Size UpdateSize(Size size)
    {
        if (this->size != size)
            MarkForRepaint();
        this->size = size;
        return this->size;
    }

  private:
    Size size;
    Vec2 position;
};
} // namespace GUI
} // namespace Vultr
