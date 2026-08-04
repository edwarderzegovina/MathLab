#include "DatasetPanel.h"

#include <cfloat>
#include <cstring>
#include <vector>

#include "imgui.h"

#include "Dataset.h"
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

void DatasetPanel::render(int id, const std::function<void(const std::string&)>& errorSink) {
    if (id != lastId_) {
        lastId_ = id;
        newValue_ = 0.0;
        std::memset(importPathBuffer_, 0, sizeof(importPathBuffer_));
        std::memset(exportPathBuffer_, 0, sizeof(exportPathBuffer_));
    }

    std::string name;
    bool found = false;
    guarded(errorSink, [&] {
        name = Workspace::instance().datasetById(id)->getName();
        found = true;
    });
    if (!found) {
        ImGui::TextDisabled("Dataset is no longer available.");
        return;
    }

    ImGui::Text("Dataset #%d \"%s\"", id, name.c_str());
    ImGui::Separator();

    renderSummary(id, errorSink);
    ImGui::Separator();
    renderTable(id, errorSink);
    ImGui::Separator();
    renderHistogram(id, errorSink);
    ImGui::Separator();
    renderActions(id, errorSink);
}

void DatasetPanel::renderSummary(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Summary");

    DatasetSummary summary;
    bool found = false;
    guarded(errorSink, [&] {
        summary = Workspace::instance().summariseDataset(id);
        found = true;
    });
    if (!found) return;

    ImGui::Text("Size: %zu", summary.size);
    ImGui::Text("Unit: %c", summary.unit);
    ImGui::Text("Sorted: %s", summary.sorted ? "yes" : "no");
    if (summary.empty) {
        // min/max/median throw on an empty set (DatasetSummary.h), so they
        // are not meaningful here -- summary.empty says so instead of
        // inventing zeros.
        ImGui::TextDisabled("(empty set - min/max/mean/median/stddev not available)");
        return;
    }
    ImGui::Text("Min: %g", summary.min);
    ImGui::Text("Max: %g", summary.max);
    ImGui::Text("Mean: %g", summary.mean);
    ImGui::Text("Median: %g", summary.median);
    ImGui::Text("Std dev: %g", summary.stddev);
}

void DatasetPanel::renderTable(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Values");

    DatasetSummary summary;
    guarded(errorSink, [&] { summary = Workspace::instance().summariseDataset(id); });
    if (summary.empty || summary.size == 0) {
        ImGui::TextDisabled("(no values)");
        return;
    }

    if (ImGui::BeginTable("datasetValues", 2,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                               ImGuiTableFlags_ScrollY,
                           ImVec2(0, 150))) {
        ImGui::TableSetupColumn("Index");
        ImGui::TableSetupColumn("Value");
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < summary.size; ++i) {
            double value = 0.0;
            guarded(errorSink, [&] { value = Workspace::instance().datasetElement(id, i); });
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%zu", i);
            ImGui::TableNextColumn();
            ImGui::Text("%g", value);
        }
        ImGui::EndTable();
    }
}

void DatasetPanel::renderHistogram(int id,
                                    const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Histogram (raw values)");

    DatasetSummary summary;
    guarded(errorSink, [&] { summary = Workspace::instance().summariseDataset(id); });
    if (summary.empty || summary.size == 0) {
        ImGui::TextDisabled("(no values)");
        return;
    }

    std::vector<float> samples;
    samples.reserve(summary.size);
    guarded(errorSink, [&] {
        for (size_t i = 0; i < summary.size; ++i)
            samples.push_back(static_cast<float>(Workspace::instance().datasetElement(id, i)));
    });
    if (samples.empty()) return;

    ImGui::PlotHistogram("##datasetHistogram", samples.data(), static_cast<int>(samples.size()),
                          0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 120));
}

void DatasetPanel::renderActions(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Actions");

    if (ImGui::Button("Sort")) {
        guarded(errorSink, [&] { Workspace::instance().datasetSort(id); });
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove outliers")) {
        guarded(errorSink, [&] { Workspace::instance().datasetRemoveOutliers(id); });
    }

    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputDouble("New value", &newValue_);
    ImGui::SameLine();
    if (ImGui::Button("Add value")) {
        const double value = newValue_;
        guarded(errorSink, [&] { Workspace::instance().datasetAddValue(id, value); });
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("CSV export");
    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputText("Export path", exportPathBuffer_, sizeof(exportPathBuffer_));
    ImGui::SameLine();
    if (ImGui::Button("Export")) {
        const std::string path = exportPathBuffer_;
        guarded(errorSink, [&] { Workspace::instance().datasetExportToCSV(id, path); });
    }

    ImGui::TextUnformatted("CSV import (creates a new dataset)");
    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputText("Import path", importPathBuffer_, sizeof(importPathBuffer_));
    ImGui::SameLine();
    if (ImGui::Button("Import")) {
        const std::string path = importPathBuffer_;
        guarded(errorSink, [&] {
            Dataset imported = Workspace::instance().datasetImportFromCSV(path);
            Workspace::instance().add(new Dataset(imported));
        });
    }
}
