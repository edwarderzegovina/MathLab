#include "MatrixPanel.h"

#include <sstream>
#include <vector>

#include "imgui.h"

#include "MathLabException.h"
#include "Workspace.h"

namespace {

// Runs a Workspace call that may throw MathLabException/std::exception and
// routes any failure through errorSink instead of letting it escape (same
// pattern as EntityList::guarded).
template <typename Fn>
void guarded(const std::function<void(const std::string&)>& errorSink, Fn&& fn) {
    try {
        fn();
    } catch (const MathLabException& e) {
        errorSink(e.getFullMessage());
    } catch (const std::exception& e) {
        errorSink(e.what());
    }
}

} // namespace

void MatrixPanel::render(int id, const std::function<void(const std::string&)>& errorSink) {
    if (id != lastId_) {
        lastId_ = id;
        otherId_ = -1;
        opIndex_ = 0;
        powerValue_ = 2;
        resultLabel_.clear();
        resultText_.clear();
        resultMatrix_.reset();
    }

    int rows = 0, columns = 0;
    std::string name;
    bool found = false;
    guarded(errorSink, [&] {
        const Matrix* current = Workspace::instance().matrixById(id);
        rows = current->getRows();
        columns = current->getColumns();
        name = current->getName();
        found = true;
    });
    if (!found) {
        ImGui::TextDisabled("Matrix is no longer available.");
        return;
    }

    ImGui::Text("Matrix #%d \"%s\"", id, name.c_str());
    ImGui::Text("Dimensions: %d x %d", rows, columns);
    ImGui::Separator();

    renderGrid(id, errorSink);
    ImGui::Separator();
    renderUnaryOps(id, errorSink);
    ImGui::Separator();
    renderBinaryOp(id, errorSink);
    ImGui::Separator();
    renderResult(errorSink);
}

void MatrixPanel::renderGrid(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Cells");

    int rows = 0, columns = 0;
    guarded(errorSink, [&] {
        const Matrix* current = Workspace::instance().matrixById(id);
        rows = current->getRows();
        columns = current->getColumns();
    });
    if (rows <= 0 || columns <= 0) return;

    if (ImGui::BeginTable("matrixGrid", columns,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
        for (int r = 0; r < rows; ++r) {
            ImGui::TableNextRow();
            for (int c = 0; c < columns; ++c) {
                ImGui::TableNextColumn();
                const size_t index =
                    static_cast<size_t>(r) * static_cast<size_t>(columns) + static_cast<size_t>(c);

                float value = 0.0f;
                guarded(errorSink,
                        [&] { value = Workspace::instance().matrixElement(id, index); });

                ImGui::PushID(static_cast<int>(index));
                ImGui::SetNextItemWidth(70.0f);
                if (ImGui::InputFloat("##cell", &value, 0.0f, 0.0f, "%.4g")) {
                    guarded(errorSink, [&] {
                        Workspace::instance().matrixSetElement(id, index, value);
                    });
                }
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
}

void MatrixPanel::renderUnaryOps(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Operations");

    if (ImGui::Button("Determinant")) {
        guarded(errorSink, [&] {
            const float det = Workspace::instance().matrixDeterminant(id);
            std::ostringstream os;
            os << det;
            resultLabel_ = "Determinant";
            resultText_ = os.str();
            resultIsMatrix_ = false;
            resultMatrix_.reset();
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Echelon form")) {
        guarded(errorSink, [&] {
            Matrix echelon = Workspace::instance().matrixEchelon(id);
            resultLabel_ = "Echelon form";
            resultText_ = echelon.toString();
            resultIsMatrix_ = true;
            resultMatrix_ = std::make_unique<Matrix>(echelon);
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Negate")) {
        guarded(errorSink, [&] {
            Matrix negated = Workspace::instance().matrixNegate(id);
            resultLabel_ = "Negated";
            resultText_ = negated.toString();
            resultIsMatrix_ = true;
            resultMatrix_ = std::make_unique<Matrix>(negated);
        });
    }

    ImGui::SetNextItemWidth(80.0f);
    ImGui::InputInt("Exponent", &powerValue_);
    ImGui::SameLine();
    if (ImGui::Button("Power")) {
        guarded(errorSink, [&] {
            Matrix powered = Workspace::instance().matrixPower(id, powerValue_);
            resultLabel_ = "Power";
            resultText_ = powered.toString();
            resultIsMatrix_ = true;
            resultMatrix_ = std::make_unique<Matrix>(powered);
        });
    }
}

void MatrixPanel::renderBinaryOp(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Binary operation with another matrix");

    std::vector<Matrix*> matrices;
    guarded(errorSink, [&] { matrices = Workspace::instance().ofType<Matrix>(); });

    const std::string preview =
        (otherId_ >= 0) ? ("#" + std::to_string(otherId_)) : "choose a matrix...";
    if (ImGui::BeginCombo("Other matrix", preview.c_str())) {
        for (const Matrix* m : matrices) {
            if (m->getId() == id) continue;
            const bool selected = (m->getId() == otherId_);
            const std::string label = "#" + std::to_string(m->getId()) + " " + m->getName();
            if (ImGui::Selectable(label.c_str(), selected)) otherId_ = m->getId();
        }
        ImGui::EndCombo();
    }

    static const char* opNames[] = {"+", "-", "*", "concat"};
    ImGui::Combo("Operation##matrixBinary", &opIndex_, opNames, IM_ARRAYSIZE(opNames));

    ImGui::BeginDisabled(otherId_ < 0);
    if (ImGui::Button("Compute##matrixBinary")) {
        const char opChar = (opIndex_ == 0)   ? '+'
                             : (opIndex_ == 1) ? '-'
                             : (opIndex_ == 2) ? '*'
                                                : 'c';
        const int rhsId = otherId_;
        guarded(errorSink, [&] {
            Matrix computed = Workspace::instance().matrixBinaryOp(id, rhsId, opChar);
            resultLabel_ = "Result of #" + std::to_string(id) + " " + opNames[opIndex_] + " #" +
                            std::to_string(rhsId);
            resultText_ = computed.toString();
            resultIsMatrix_ = true;
            resultMatrix_ = std::make_unique<Matrix>(computed);
        });
    }
    ImGui::EndDisabled();
}

void MatrixPanel::renderResult(const std::function<void(const std::string&)>& errorSink) {
    if (resultText_.empty()) return;

    ImGui::Separator();
    ImGui::TextUnformatted(resultLabel_.c_str());
    ImGui::TextWrapped("%s", resultText_.c_str());

    if (resultIsMatrix_ && resultMatrix_) {
        if (ImGui::Button("Add result to workspace")) {
            guarded(errorSink,
                    [&] { Workspace::instance().add(new Matrix(*resultMatrix_)); });
            resultMatrix_.reset();
            resultText_.clear();
        }
    }
}
