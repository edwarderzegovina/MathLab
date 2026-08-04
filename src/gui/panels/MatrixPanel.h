#ifndef MATHLAB_GUI_MATRIXPANEL_H
#define MATHLAB_GUI_MATRIXPANEL_H

#include <functional>
#include <memory>
#include <string>

#include "Matrix.h"

// Right-pane panel for a selected Matrix: dimensions, an editable cell grid,
// determinant/echelon/power/negate buttons, and a binary-op row against
// another matrix in the workspace.
//
// Deliberately UI-only: every number shown or written comes from a Workspace
// call (Matrix.h is included only for the type, to hold a computed result
// until "Add to workspace" is clicked -- never to compute with it). Any
// MathLabException/std::exception raised talking to the Workspace is reported
// through errorSink instead of propagating. Every action is click-triggered,
// not recomputed every frame, so it doesn't burn through the global id
// counter just by being on screen.
class MatrixPanel {
public:
    // `id` is the selected matrix's id (already known to be a Matrix by the
    // caller's dynamic_cast). Must be called between ImGui::NewFrame() and
    // ImGui::Render().
    void render(int id, const std::function<void(const std::string&)>& errorSink);

private:
    void renderGrid(int id, const std::function<void(const std::string&)>& errorSink);
    void renderUnaryOps(int id, const std::function<void(const std::string&)>& errorSink);
    void renderBinaryOp(int id, const std::function<void(const std::string&)>& errorSink);
    void renderResult(const std::function<void(const std::string&)>& errorSink);

    int lastId_ = -1;   // resets the scratch state below when the selection changes
    int powerValue_ = 2;
    int otherId_ = -1;
    int opIndex_ = 0;   // 0='+', 1='-', 2='*', 3=concat

    // The most recently computed result (determinant/echelon/power/negate/
    // binary op), kept alive so "Add to workspace" can commit it without
    // recomputing. Reset whenever a new op runs.
    std::string resultLabel_;
    std::string resultText_;
    bool resultIsMatrix_ = false;
    std::unique_ptr<Matrix> resultMatrix_;
};

#endif // MATHLAB_GUI_MATRIXPANEL_H
