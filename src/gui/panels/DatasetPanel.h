#ifndef MATHLAB_GUI_DATASETPANEL_H
#define MATHLAB_GUI_DATASETPANEL_H

#include <functional>
#include <string>

// Right-pane panel for a selected Dataset: a scrollable value table, the
// DatasetSummary fields, a histogram of the raw values, and buttons for
// sort, add value, remove outliers, and CSV import/export.
//
// The "histogram" plots the raw values themselves (one bar per element,
// straight from Workspace::datasetElement) rather than a computed
// frequency/bucket histogram: Dataset exposes no binning operation, and this
// panel only delegates to operations the entity already has rather than
// inventing new mathematics in Workspace.
//
// Deliberately UI-only: every number shown comes from a Workspace call.
class DatasetPanel {
public:
    void render(int id, const std::function<void(const std::string&)>& errorSink);

private:
    void renderSummary(int id, const std::function<void(const std::string&)>& errorSink);
    void renderTable(int id, const std::function<void(const std::string&)>& errorSink);
    void renderHistogram(int id, const std::function<void(const std::string&)>& errorSink);
    void renderActions(int id, const std::function<void(const std::string&)>& errorSink);

    int lastId_ = -1;

    double newValue_ = 0.0;
    char importPathBuffer_[256] = "";
    char exportPathBuffer_[256] = "";
};

#endif // MATHLAB_GUI_DATASETPANEL_H
