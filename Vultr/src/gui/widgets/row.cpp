#include <gui/widgets/row.h>
#include <gui/widgets/align.h>

using namespace Vultr;

#define __row_cache_id ui_id(__FILE__)

#define DEFAULT_ROW_CHILDREN_SIZE 10

template <>
void IMGUI::destroy_cache<IMGUI::RowState>(Context *c, RowState &cache)
{
    // If we can, then free the widget order array
    if (cache.widget_order != nullptr)
        free(cache.widget_order);
};

static void expand_widget_order_array(IMGUI::RowState &state)
{
    using namespace IMGUI;

    // We will expand by adding the default row children size to our array
    state.widget_order_size += DEFAULT_ROW_CHILDREN_SIZE + state.index - state.widget_order_size;
    state.widget_order = static_cast<UI_ID *>(realloc(state.widget_order, sizeof(UI_ID) * state.widget_order_size));
}

void IMGUI::begin_row(Context *c, UI_ID id, RowStyle style)
{
    // Load our style into cache
    auto &state = get_widget_cache<RowState>(c, __row_cache_id, id);
    state.style = style;

    // If we haven't created our array of widget IDs yet, then do so here
    if (state.widget_order == nullptr)
    {
        // Create our array of UI_IDs which show the order. This will be the default row children size so that we don't hammer the heap with allocations unnecessarily for each new widget.
        // This array can always be expanded later
        state.widget_order_size = DEFAULT_ROW_CHILDREN_SIZE;
        state.widget_order = static_cast<UI_ID *>(malloc(sizeof(UI_ID) * state.widget_order_size));
    }

    // Reset other state values for next run
    state.index = -1;
    state.total_width = 0;
    state.max_height = 0;
    state.flex_accumulator = 0;

    // Always make sure to reset all of the values in our array of widget IDs each render, even if we don't alloc
    for (size_t i = 0; i < state.widget_order_size; i++)
    {
        state.widget_order[i] = UNSET_ID;
    }

    // Get our parent constraints
    Constraints constraints = get_constraints(c, id);

    // Get the available width
    state.available_width = constraints.max_width;

    // We are going to generate loose constraints for everything. All children will be given by an unbounded width. The max height will just be what the parent gives us
    constraints.min_width = 0;
    constraints.max_width = UNBOUNDED;

    // If the cross axis alignment is stretch, then we will use a tight constraint for our height
    if (style.cross_axis_alignment == CrossAxisAlignment::STRETCH)
    {
        constraints.min_height = constraints.max_height;
    }
    // Otherwise it will be loose
    else
    {
        constraints.min_height = 0;
    }

    // Create our layout
    auto mcl = new_multi_child_layout(__row_cache_id, id, Vec2(0), constraints, Vec2(0));

    // Begin our layout
    begin_layout_with_children(c, id, mcl);
}

#define __row_element_cache_id __row_cache_id + ui_id("row_element")

void IMGUI::begin_row_element(Context *c, s32 index, f32 flex_factor)
{
    // Get the row and row layout
    UI_ID row = c->parent;
    Layout &layout = get_widget_layout(c, row);
    MultiChildLayout &data = get_layout_data<MultiChildLayout>(layout);

    // Make sure that our current parent is actually a row widget
    assert(layout.widget_type == __row_cache_id && "Row element not inside a row widget!");

    // Get our cache
    auto &state = get_widget_cache<RowState>(c, __row_cache_id, row);

    // If the index is not specified, we will automatically just tick up
    if (index == -1)
    {
        state.index++;
    }
    else
    {
        state.index = index;
    }

    // Validate our input
    assert((state.index == -1 || state.index >= 0) && "Index must be >= 0!");

    // If there isn't room for this index, then make room
    if (state.index >= state.widget_order_size)
    {
        expand_widget_order_array(state);
    }

    Constraints constraints = data.default_constraints;

    // If no flex factor is specified then it will be assumed that the widget will
    if (flex_factor == -1)
    {
        assert(state.flex_accumulator == 0 && "All row elements with a flex factor must be created after all other elements!");
    }
    else
    {
        assert(flex_factor <= 1 && flex_factor >= 0 && "The flex factor must be a percentage between 0-1!");

        // Accumulate our flex factor
        state.flex_accumulator += flex_factor;

        // Get the remaining space
        f32 remaining_space = state.available_width - state.total_width;

        // The flex factor is a percentage, so we will multiply the flex factor by the remaining space to get this
        f32 width = remaining_space * flex_factor;

        // Create tight constraints for the width
        constraints.min_width = width;
        constraints.max_width = width;
    }

    // ID for our single child layout
    UI_ID id = row + __row_element_cache_id + __LINE__ + state.index;

    // Create a single child layout which will hold a child widget
    Layout scl = new_single_child_layout(__row_element_cache_id, id, Vec2(0), constraints);

    // Begin our layout
    begin_layout_with_children(c, id, scl);
}

void IMGUI::end_row_element(Context *c)
{
    // End our layout
    Layout &layout = end_layout_with_children(c, __row_element_cache_id);
    SingleChildLayout &data = get_layout_data<SingleChildLayout>(layout);
    UI_ID id = layout.owner;
    UI_ID row = layout.parent;
    UI_ID child = data.child;

    // Verify that a child was actually attached
    assert(child != NO_ID && child != UNSET_ID && "A child must be added to a row element!");

    // Get the child layout
    Layout child_layout = get_widget_layout(c, child);

    // Make sure that the child does not have infinite size
    assert(child_layout.local_size.x != UNBOUNDED && "Widget with infinite width cannot be placed inside of a row!");

    // Get our cache
    auto &state = get_widget_cache<RowState>(c, __row_cache_id, row);

    // Add the UI_ID of our child to our widget order
    state.widget_order[state.index] = id;

    // Set our size to match the size of our child
    Constraints parent_constraints = get_constraints(c, id);
    layout.local_size = get_size_from_constraints(parent_constraints, child_layout.local_size);

    // Update our total width
    if (state.flex_accumulator == 0)
    {
        state.total_width += layout.local_size.x;
    }

    // Update our max height
    if (layout.local_size.y > state.max_height)
    {
        state.max_height = layout.local_size.y;
    }
}

void IMGUI::end_row(Context *c)
{
    // End our layout
    Layout &layout = end_layout_with_children(c, __row_cache_id);
    MultiChildLayout &data = get_layout_data<MultiChildLayout>(layout);
    UI_ID id = layout.owner;

    // Get our cache
    auto &state = get_widget_cache<RowState>(c, __row_cache_id, id);
    RowStyle &style = state.style;

    assert((state.flex_accumulator == 1 || state.flex_accumulator == 0) && "Flex must accumulate to 1!");

    // Verify that all of our children are row elements
    for (auto child : data.children)
    {
        // Get the child layout
        Layout &child_layout = get_widget_layout(c, child);

        // Verify that the child is a row element
        assert(child_layout.widget_type == __row_element_cache_id && "All children of a row must be wrapped with a `begin_row_element` and `end_row_element`");
    }

    // Get the parent constraints
    Constraints parent_constraints = get_constraints(c, id);

    // Set our local size
    if (style.main_axis_size == MainAxisSize::MAX || state.flex_accumulator != 0)
    {
        layout.local_size = Vec2(parent_constraints.max_width, state.max_height);
    }
    else
    {
        layout.local_size = Vec2(state.total_width, state.max_height);
    }

    // Make sure that our local size is conforming to the constraints
    layout.local_size = get_size_from_constraints(parent_constraints, layout.local_size);

    // Loop through all of the UI_IDs in our ordered array
    f32 cursor = 0;

    // Additional space to be added between row elements
    f32 cursor_increment = 0;

    // If there is any flex, then main axis alignment makes 0 difference, the cursor will always start at the beginning
    // If main axis size is min, then there is no remaining space so the cursor will also always start at the beginning
    if (state.flex_accumulator == 0 && style.main_axis_size != MainAxisSize::MIN)
    {
        // Get the alignment
        auto mal = style.main_axis_alignment;

        // Find the remaining space based on our constraints and size
        f32 remaining_space = parent_constraints.max_width - state.total_width;

        if (mal == MainAxisAlignment::START)
        {
            if (style.horizontal_direction == HorizontalDirection::LEFT_TO_RIGHT)
            {
                // If the horizontal direction is left to right, then the start is at 0, the left of the row
                cursor = 0;
            }
            else
            {
                // If the horizontal direction is right to left, then the start is at the remaining_space, the right of the row
                cursor = remaining_space;
            }
        }
        else if (mal == MainAxisAlignment::CENTER)
        {
            cursor = remaining_space / 2;
        }
        else if (mal == MainAxisAlignment::END)
        {
            if (style.horizontal_direction == HorizontalDirection::LEFT_TO_RIGHT)
            {
                // If the horizontal direction is left to right, then the end is at the remaining_space, the right of the row
                cursor = remaining_space;
            }
            else
            {
                // If the horizontal direction is right to left, then the end is at 0, the left of the row
                cursor = 0;
            }
        }
        else if (mal == MainAxisAlignment::SPACE_EVENLY)
        {
            // For space evenly, we will have num_children + 1 empty spaces so we will distribute the remaining space among that number
            f32 space = remaining_space / (data.children.size() + 1);
            cursor_increment = space;
            cursor = space;
        }
        else if (mal == MainAxisAlignment::SPACE_BETWEEN)
        {
            // For space between, we will have num_children - 1 empty spaces so we will distribute the remaining space among that number
            f32 space = remaining_space / (data.children.size() - 1);
            cursor_increment = space;
            cursor = 0;
        }
        else if (mal == MainAxisAlignment::SPACE_AROUND)
        {
            // For space around, we will have num_children empty spaces (the last space will be divided by two which will be placed at the beginning and end
            // so that the edges are only half the size as the rest of the spaces) so we will distribute the remaining space among that number
            f32 space = remaining_space / data.children.size();
            cursor_increment = space;
            cursor = space / 2;
        }
    }

    for (size_t i = 0; i < state.widget_order_size; i++)
    {
        // Get the child UI_ID
        UI_ID child = state.widget_order[i];

        // If the child is non existant, then we should do nothing. This will allow users to put "holes" in the row and it will still work fine which allows for more flexibility
        if (child == NO_ID || child == UNSET_ID)
            continue;

        // Get the child layout
        Layout child_layout = get_widget_layout(c, child);

        // Convert our cross axis alignment to a real alignment.
        // We only really care about the Y value in this alignment so we will just use a left alignment for everything
        Alignment cross_axis_alignment;
        switch (style.cross_axis_alignment)
        {
            case CrossAxisAlignment::STRETCH:
            case CrossAxisAlignment::START:
                cross_axis_alignment = Alignment::TOP_LEFT();
                break;
            case CrossAxisAlignment::CENTER:
                cross_axis_alignment = Alignment::CENTER_LEFT();
                break;
            case CrossAxisAlignment::END:
                cross_axis_alignment = Alignment::BOTTOM_LEFT();
                break;
        }

        // We will also respect direction. By default this will be going down, but if it is up then invert all the Y values of our alignments
        if (style.vertical_direction == VerticalDirection::UP)
        {
            cross_axis_alignment.value *= Vec2(1, -1);
        }

        // Align each of the widgets vertically inside the row
        Vec2 position = get_alignment_position(cross_axis_alignment, child_layout.local_size, layout.local_size);

        // Update the child's position in the transforms. Only care about the Y position
        data.children_transforms[child].position = Vec2(cursor, position.y);
        cursor += child_layout.local_size.x + cursor_increment;
    }
}
