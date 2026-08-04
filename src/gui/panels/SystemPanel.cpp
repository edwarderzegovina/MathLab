#include "SystemPanel.h"

#include <sstream>
#include <vector>

#include "imgui.h"

#include "MathLabException.h"
#include "Workspace.h"

namespace {

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

void SystemPanel::render(int id, const std::function<void(const std::string&)>& errorSink) {
    if (id != lastId_) {
        lastId_ = id;
        hasSolved_ = false;
        inconsistent_ = false;
        inconsistentMessage_.clear();
        otherId_ = -1;
        opIndex_ = 0;
        resultLabel_.clear();
        resultText_.clear();
        resultSystem_.reset();
    }

    std::string rendered, name;
    bool found = false;
    guarded(errorSink, [&] {
        const LinearSystem* current = Workspace::instance().systemById(id);
        rendered = current->toString();
        name = current->getName();
        found = true;
    });
    if (!found) {
        ImGui::TextDisabled("System is no longer available.");
        return;
    }

    ImGui::Text("LinearSystem #%d \"%s\"", id, name.c_str());
    ImGui::TextWrapped("%s", rendered.c_str());
    ImGui::Separator();

    renderFreeTerms(id, errorSink);
    ImGui::Separator();
    renderSolve(id, rendered, errorSink);
    ImGui::Separator();
    renderBinaryOp(id, errorSink);
}

void SystemPanel::renderFreeTerms(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Free terms");

    int equations = 0;
    guarded(errorSink,
            [&] { equations = Workspace::instance().systemById(id)->getNrOfEquations(); });
    if (equations <= 0) return;

    for (size_t eq = 0; eq < static_cast<size_t>(equations); ++eq) {
        float freeTerm = 0.0f;
        guarded(errorSink,
                [&] { freeTerm = Workspace::instance().systemFreeTerm(id, eq); });
        ImGui::Text("Equation %zu: b = %g", eq + 1, static_cast<double>(freeTerm));
    }
}

void SystemPanel::renderSolve(int id, const std::string& content,
                               const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Solution");

    // Solves automatically on selection -- no click required, matching
    // DatasetPanel's histogram and PolynomialPanel's curve plot -- but only
    // when the id or the system's contents changed since the last frame:
    // nothing in this panel lets the user edit the selected system's
    // coefficients (renderFreeTerms is read-only), so `content` changing at
    // all here would mean something else in the workspace touched it.
    if (id != solveCachedId_ || content != solveCachedContent_) {
        hasSolved_ = false;
        inconsistent_ = false;
        inconsistentMessage_.clear();
        try {
            solveReport_ = Workspace::instance().solveSystem(id);
            hasSolved_ = true;
        } catch (const CalculationException& e) {
            // Documented behaviour (SolutionReport.h): an inconsistent system
            // throws instead of returning SolutionKind::Inconsistent. This is
            // the answer "no solution", not a failure, so it is rendered here
            // rather than sent through errorSink/the toast queue.
            inconsistent_ = true;
            inconsistentMessage_ = e.getFullMessage();
        } catch (const MathLabException& e) {
            errorSink(e.getFullMessage());
        } catch (const std::exception& e) {
            errorSink(e.what());
        }
        solveCachedId_ = id;
        solveCachedContent_ = content;
    }

    if (inconsistent_) {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "No solution - inconsistent system");
        ImGui::TextWrapped("%s", inconsistentMessage_.c_str());
        return;
    }

    if (!hasSolved_) return;

    if (solveReport_.kind == SolutionKind::Unique) {
        ImGui::TextUnformatted(solveReport_.message.c_str());
        if (ImGui::BeginTable("uniqueSolution", 2,
                               ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Variable");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            for (size_t j = 0; j < solveReport_.values.size(); ++j) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(
                    j < solveReport_.variableNames.size() ? solveReport_.variableNames[j].c_str()
                                                            : "?");
                ImGui::TableNextColumn();
                ImGui::Text("%g", static_cast<double>(solveReport_.values[j]));
            }
            ImGui::EndTable();
        }
    } else if (solveReport_.kind == SolutionKind::Infinite) {
        ImGui::TextUnformatted(solveReport_.message.c_str());
        ImGui::TextUnformatted("Free parameters:");
        for (const auto& line : solveReport_.freeParameters)
            ImGui::BulletText("%s", line.c_str());
        ImGui::TextUnformatted("Parametrised solution:");
        for (size_t j = 0; j < solveReport_.parametrised.size(); ++j) {
            const std::string var =
                j < solveReport_.variableNames.size() ? solveReport_.variableNames[j] : "?";
            ImGui::BulletText("%s = %s", var.c_str(), solveReport_.parametrised[j].c_str());
        }
    } else {
        // Not reachable in practice (see the class comment) but rendered
        // rather than silently dropped if it ever is.
        ImGui::TextWrapped("%s", solveReport_.message.c_str());
    }
}

void SystemPanel::renderBinaryOp(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Binary operation with another system");

    std::vector<LinearSystem*> systems;
    guarded(errorSink, [&] { systems = Workspace::instance().ofType<LinearSystem>(); });

    const std::string preview =
        (otherId_ >= 0) ? ("#" + std::to_string(otherId_)) : "choose a system...";
    if (ImGui::BeginCombo("Other system", preview.c_str())) {
        for (const LinearSystem* s : systems) {
            if (s->getId() == id) continue;
            const bool selected = (s->getId() == otherId_);
            const std::string label = "#" + std::to_string(s->getId()) + " " + s->getName();
            if (ImGui::Selectable(label.c_str(), selected)) otherId_ = s->getId();
        }
        ImGui::EndCombo();
    }

    static const char* opNames[] = {"+", "-"};
    ImGui::Combo("Operation##systemBinary", &opIndex_, opNames, IM_ARRAYSIZE(opNames));

    ImGui::BeginDisabled(otherId_ < 0);
    if (ImGui::Button("Compute##systemBinary")) {
        const char opChar = (opIndex_ == 0) ? '+' : '-';
        const int rhsId = otherId_;
        guarded(errorSink, [&] {
            LinearSystem computed = Workspace::instance().systemBinaryOp(id, rhsId, opChar);
            resultLabel_ = "Result of #" + std::to_string(id) + " " + opNames[opIndex_] + " #" +
                            std::to_string(rhsId);
            resultText_ = computed.toString();
            resultSystem_ = std::make_unique<LinearSystem>(computed);
        });
    }
    ImGui::EndDisabled();

    if (!resultText_.empty()) {
        ImGui::TextUnformatted(resultLabel_.c_str());
        ImGui::TextWrapped("%s", resultText_.c_str());
        if (resultSystem_) {
            if (ImGui::Button("Add result to workspace")) {
                guarded(errorSink, [&] {
                    Workspace::instance().add(new LinearSystem(*resultSystem_));
                });
                resultSystem_.reset();
                resultText_.clear();
            }
        }
    }
}
