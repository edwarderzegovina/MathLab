#ifndef MATHLAB_GUI_ENTITYLIST_H
#define MATHLAB_GUI_ENTITYLIST_H

#include <functional>
#include <string>

// Left-pane panel: lists every MathEntity currently in the Workspace, with a
// type filter, click-to-select, a Delete button and a Sort-by-name button.
//
// Deliberately UI-only glue around Workspace: every call that can throw a
// MathLabException is wrapped here and reported through the errorSink
// callback (GuiApp::showError) rather than being allowed to escape. No
// arithmetic, comparison, parsing or formatting of entity CONTENT happens in
// this class -- it only asks Workspace for entities and renders what they
// already report via getId()/getName()/toString().
class EntityList {
public:
    enum class TypeFilter { All, Matrix, Polynomial, LinearSystem, Dataset };

    // Renders the panel (must be called between ImGui::NewFrame() and
    // ImGui::Render()). Any MathLabException/std::exception raised while
    // talking to the Workspace is reported through errorSink instead of
    // propagating.
    void render(const std::function<void(const std::string&)>& errorSink);

    // The id of the currently selected entity, or -1 if none is selected.
    int selectedId() const { return selectedId_; }

private:
    TypeFilter filter_ = TypeFilter::All;
    int selectedId_ = -1;
};

#endif // MATHLAB_GUI_ENTITYLIST_H
