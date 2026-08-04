#include "EntityList.h"

#include <cstdio>
#include <vector>

#include "imgui.h"

#include "Dataset.h"
#include "LinearSystem.h"
#include "MathEntity.h"
#include "MathLabException.h"
#include "Matrix.h"
#include "Polynomial.h"
#include "Workspace.h"

namespace {

// Pure UI classification (RTTI, not arithmetic) so the list can show a type
// column and the filter can group entities -- Workspace already exposes
// typed accessors (matrixById, ofType<Matrix>(), ...) for anything that
// actually needs to compute with a specific type.
const char* typeLabel(const MathEntity* entity) {
    if (dynamic_cast<const Matrix*>(entity) != nullptr) return "Matrix";
    if (dynamic_cast<const Polynomial*>(entity) != nullptr) return "Polynomial";
    if (dynamic_cast<const LinearSystem*>(entity) != nullptr) return "LinearSystem";
    if (dynamic_cast<const Dataset*>(entity) != nullptr) return "Dataset";
    return "Unknown";
}

bool passesFilter(const MathEntity* entity, EntityList::TypeFilter filter) {
    switch (filter) {
        case EntityList::TypeFilter::All: return true;
        case EntityList::TypeFilter::Matrix: return dynamic_cast<const Matrix*>(entity) != nullptr;
        case EntityList::TypeFilter::Polynomial: return dynamic_cast<const Polynomial*>(entity) != nullptr;
        case EntityList::TypeFilter::LinearSystem: return dynamic_cast<const LinearSystem*>(entity) != nullptr;
        case EntityList::TypeFilter::Dataset: return dynamic_cast<const Dataset*>(entity) != nullptr;
    }
    return true;
}

// Runs a Workspace call that may throw MathLabException/std::exception and
// routes any failure through errorSink instead of letting it escape.
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

void EntityList::render(const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Entities");
    ImGui::Separator();

    static const char* filterNames[] = {"All", "Matrix", "Polynomial", "LinearSystem", "Dataset"};
    int filterIndex = static_cast<int>(filter_);
    if (ImGui::Combo("Filter", &filterIndex, filterNames, IM_ARRAYSIZE(filterNames))) {
        filter_ = static_cast<TypeFilter>(filterIndex);
    }

    if (ImGui::Button("Sort by name")) {
        guarded(errorSink, [&] { Workspace::instance().sortByName(); });
    }
    ImGui::SameLine();
    const bool hasSelection = selectedId_ >= 0;
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button("Delete selected")) {
        const int idToDelete = selectedId_;
        guarded(errorSink, [&] { Workspace::instance().remove(idToDelete); });
        selectedId_ = -1;
    }
    ImGui::EndDisabled();

    std::vector<MathEntity*> entities;
    guarded(errorSink, [&] { entities = Workspace::instance().all(); });

    if (ImGui::BeginTable("entityTable", 3,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                           ImGuiTableFlags_ScrollY,
                           ImVec2(0, -1))) {
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableHeadersRow();

        for (const MathEntity* entity : entities) {
            if (!passesFilter(entity, filter_)) continue;
            const int id = entity->getId();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            const bool isSelected = (id == selectedId_);
            char selectableLabel[32];
            std::snprintf(selectableLabel, sizeof(selectableLabel), "%s##%d", typeLabel(entity), id);
            if (ImGui::Selectable(selectableLabel, isSelected,
                                   ImGuiSelectableFlags_SpanAllColumns)) {
                selectedId_ = id;
            }
            ImGui::TableNextColumn();
            ImGui::Text("%d", id);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(entity->getName().c_str());
        }

        ImGui::EndTable();
    }
}
