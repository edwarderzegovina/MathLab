#include "GuiApp.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>

#include "imgui.h"

#include "Dataset.h"
#include "LinearSystem.h"
#include "Logger.h"
#include "MathEntity.h"
#include "MathLabException.h"
#include "Matrix.h"
#include "Polynomial.h"
#include "Workspace.h"

GuiApp::GuiApp() = default;

void GuiApp::showError(const std::string& message) {
    toasts_.push_back(Toast{message, 5.0f});
}

void GuiApp::loadWorkspaceDirectory(const std::string& directory) {
    try {
        Workspace::instance().loadFromDirectory(directory);
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

void GuiApp::saveWorkspaceDirectory(const std::string& directory) {
    try {
        Workspace::instance().saveToDirectory(directory);
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }
}

void GuiApp::renderMenuBar() {
    bool openLoadPopup = false;
    bool openSavePopup = false;
    bool openAboutPopup = false;
    bool openNewMatrixPopup = false;
    bool openNewPolynomialPopup = false;
    bool openNewSystemPopup = false;
    bool openNewDatasetPopup = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load workspace...")) openLoadPopup = true;
            if (ImGui::MenuItem("Save workspace...")) openSavePopup = true;
            if (ImGui::MenuItem("Load sample data")) loadWorkspaceDirectory("data");
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) quitRequested_ = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("New")) {
            if (ImGui::MenuItem("Matrix...")) openNewMatrixPopup = true;
            if (ImGui::MenuItem("Polynomial...")) openNewPolynomialPopup = true;
            if (ImGui::MenuItem("Linear system...")) openNewSystemPopup = true;
            if (ImGui::MenuItem("Dataset...")) openNewDatasetPopup = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) openAboutPopup = true;
            if (ImGui::MenuItem("View log", nullptr, showLogWindow_)) showLogWindow_ = !showLogWindow_;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (openLoadPopup) ImGui::OpenPopup("Load workspace");
    if (openSavePopup) ImGui::OpenPopup("Save workspace");
    if (openAboutPopup) ImGui::OpenPopup("About MathLab");
    if (openNewMatrixPopup) ImGui::OpenPopup("New Matrix");
    if (openNewPolynomialPopup) ImGui::OpenPopup("New Polynomial");
    if (openNewSystemPopup) ImGui::OpenPopup("New Linear System");
    if (openNewDatasetPopup) ImGui::OpenPopup("New Dataset");

    if (ImGui::BeginPopupModal("Load workspace", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Directory to load matrices.txt / polynomials.txt /");
        ImGui::TextUnformatted("systems.txt / datasets.txt from:");
        ImGui::InputText("##loadDir", loadDirectoryBuffer_, sizeof(loadDirectoryBuffer_));
        if (ImGui::Button("Load")) {
            loadWorkspaceDirectory(loadDirectoryBuffer_);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Save workspace", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Directory to save the whole workspace into:");
        ImGui::InputText("##saveDir", saveDirectoryBuffer_, sizeof(saveDirectoryBuffer_));
        if (ImGui::Button("Save")) {
            saveWorkspaceDirectory(saveDirectoryBuffer_);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("About MathLab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("MathLab");
        ImGui::TextUnformatted("A GUI front-end over the UI-free Workspace core.");
        ImGui::TextUnformatted("Matrices, polynomials, linear systems and data sets.");
        if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    renderNewEntityPopups();
}

// The four "New ..." modals. Each Create button calls the matching private
// creation helper below.
void GuiApp::renderNewEntityPopups() {
    if (ImGui::BeginPopupModal("New Matrix", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Creates a zero matrix of the given size.");
        ImGui::TextUnformatted("(maximum 10x10 -- Matrix::MAX_SIZE)");
        ImGui::InputInt("Rows", &newMatrixRows_);
        ImGui::InputInt("Columns", &newMatrixColumns_);
        if (ImGui::Button("Create##matrix")) {
            if (createMatrix(newMatrixRows_, newMatrixColumns_)) ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##matrix")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("New Polynomial", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Same syntax the console front-end reads, e.g.:");
        ImGui::BulletText("3x^2+1   (the '*' is optional)");
        ImGui::BulletText("3*x^2+1");
        ImGui::BulletText("2x");
        ImGui::BulletText("-4x^3+2");
        ImGui::TextUnformatted("No spaces -- it is read as a single token, just like the CLI.");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("##polyText", newPolynomialBuffer_, sizeof(newPolynomialBuffer_));
        if (ImGui::Button("Create##poly")) {
            if (createPolynomial(newPolynomialBuffer_)) ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##poly")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("New Linear System", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Variables <= 9, equations <= 10 (augmented matrix is");
        ImGui::TextUnformatted("equations x (variables+1), and Matrix::MAX_SIZE is 10).");
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("Name", newSystemNameBuffer_, sizeof(newSystemNameBuffer_));
        ImGui::InputInt("Variables", &newSystemVariables_);
        ImGui::InputInt("Equations", &newSystemEquations_);
        ImGui::TextUnformatted("Equations, e.g. 3*y=6  or  1*x+2*y+3*z=5 (name-based placement):");
        const int shownRows = std::min(std::max(newSystemEquations_, 0), kMaxSystemEquationRows);
        for (int i = 0; i < shownRows; ++i) {
            ImGui::PushID(i);
            ImGui::SetNextItemWidth(300.0f);
            const std::string label = "Eq " + std::to_string(i + 1);
            ImGui::InputText(label.c_str(), newSystemEquationBuffers_[static_cast<size_t>(i)].data(),
                              newSystemEquationBuffers_[static_cast<size_t>(i)].size());
            ImGui::PopID();
        }
        if (ImGui::Button("Create##system")) {
            std::vector<std::string> lines;
            for (int i = 0; i < shownRows; ++i)
                lines.emplace_back(newSystemEquationBuffers_[static_cast<size_t>(i)].data());
            if (createLinearSystem(newSystemVariables_, newSystemEquations_, newSystemNameBuffer_, lines))
                ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##system")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("New Dataset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Values, comma- or space-separated, e.g. 1, 2, 3.5 -7");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("##datasetText", newDatasetBuffer_, sizeof(newDatasetBuffer_));
        if (ImGui::Button("Create##dataset")) {
            if (createDataset(newDatasetBuffer_)) ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##dataset")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

// Read-only viewer over Logger's buffered text (Logger::logText() already
// returns the buffer instead of only writing to a stream -- see Logger.h --
// so this renders it rather than re-collecting log lines itself).
void GuiApp::renderLogWindow() {
    if (!showLogWindow_) return;

    ImGui::SetNextWindowSize(ImVec2(600.0f, 400.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Log", &showLogWindow_)) {
        const std::string typesLine = Logger::getInstance().uniqueTypesText();
        ImGui::TextWrapped("%s", typesLine.c_str());
        ImGui::Separator();
        const std::string text = Logger::getInstance().logText();
        ImGui::InputTextMultiline("##logText", const_cast<char*>(text.c_str()),
                                   text.size() + 1, ImVec2(-1.0f, -1.0f),
                                   ImGuiInputTextFlags_ReadOnly);
    }
    ImGui::End();
}

void GuiApp::renderInspector() {
    ImGui::TextUnformatted("Inspector");
    ImGui::Separator();

    const int id = entityList_.selectedId();
    if (id < 0) {
        ImGui::TextDisabled("No entity selected.");
        return;
    }

    // Dispatched on the selected entity's dynamic type, exactly as
    // EntityList's type column/filter already does: no arithmetic here, just
    // RTTI to pick which panel renders (each panel then talks to Workspace
    // itself).
    MathEntity* entity = nullptr;
    try {
        entity = Workspace::instance().byId(id);
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }

    if (entity == nullptr) {
        ImGui::TextDisabled("Selected entity is no longer available.");
        return;
    }

    const auto errorSink = [this](const std::string& message) { showError(message); };

    if (dynamic_cast<Matrix*>(entity) != nullptr) {
        matrixPanel_.render(id, errorSink);
    } else if (dynamic_cast<Polynomial*>(entity) != nullptr) {
        polynomialPanel_.render(id, errorSink);
    } else if (dynamic_cast<LinearSystem*>(entity) != nullptr) {
        systemPanel_.render(id, errorSink);
    } else if (dynamic_cast<Dataset*>(entity) != nullptr) {
        datasetPanel_.render(id, errorSink);
    } else {
        ImGui::TextWrapped("%s", entity->toString().c_str());
    }
}

void GuiApp::renderToasts() {
    if (toasts_.empty()) return;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float yOffset = 40.0f;
    for (size_t i = 0; i < toasts_.size(); ++i) {
        ImGui::SetNextWindowPos(
            ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 20.0f,
                   viewport->WorkPos.y + yOffset),
            ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::SetNextWindowBgAlpha(0.9f);

        char windowId[32];
        std::snprintf(windowId, sizeof(windowId), "##toast%zu", i);
        ImGui::Begin(windowId, nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error");
        ImGui::TextWrapped("%s", toasts_[i].message.c_str());
        yOffset += ImGui::GetWindowSize().y + 10.0f;
        ImGui::End();
    }
}

void GuiApp::renderFrame(float deltaSeconds) {
    renderMenuBar();

    for (auto it = toasts_.begin(); it != toasts_.end();) {
        it->secondsRemaining -= deltaSeconds;
        if (it->secondsRemaining <= 0.0f)
            it = toasts_.erase(it);
        else
            ++it;
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("MathLab", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    ImGui::BeginChild("EntityListPane", ImVec2(viewport->WorkSize.x * 0.4f, 0), true);
    entityList_.render([this](const std::string& message) { showError(message); });
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("InspectorPane", ImVec2(0, 0), true);
    renderInspector();
    ImGui::EndChild();

    ImGui::End();

    renderLogWindow();
    renderToasts();
}

// --- entity creation ------------------------------------------------------
//
// Every one of these goes through the exact operator>>/constructor the
// console front-end uses (Matrix/Polynomial/LinearSystem/Dataset's own
// istream extraction, defined in src/core, and LinearSystem::validateShape /
// Matrix's dimension check, also in src/core) -- nothing here parses a
// coefficient, a variable name or an exponent itself.

bool GuiApp::createMatrix(int rows, int columns) {
    try {
        // Matrix(rows, columns) validates the shape from its own mem-init
        // list (Matrix::validateDimensions, MAX_SIZE = 10) and throws
        // DimensionMismatchException before anything is allocated -- exactly
        // the check ConsoleApp's "Add new matrix (manual)" relies on.
        Workspace::instance().add(new Matrix(rows, columns));
        return true;
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }
    return false;
}

bool GuiApp::createPolynomial(const std::string& text) {
    try {
        if (text.find_first_not_of(" \t\r\n") == std::string::npos)
            throw ParseException("Enter a polynomial, e.g. 3x^2+1");
        // Same Polynomial::operator>> the CLI uses, just fed from a string
        // stream instead of stdin. It reads one whitespace-delimited token,
        // so the text must not contain spaces.
        Polynomial parsed;
        std::istringstream iss(text);
        iss >> parsed;
        Workspace::instance().add(new Polynomial(parsed));
        return true;
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }
    return false;
}

bool GuiApp::createLinearSystem(int variables, int equations, const std::string& name,
                                 const std::vector<std::string>& equationLines) {
    try {
        // Same pre-construction check ConsoleIO::readSystem() makes, and for
        // the same reason: a constructor that throws from its mem-init list
        // has already consumed an id the user can see.
        LinearSystem::validateShape(variables, equations);
        LinearSystem system(variables, equations, name.empty() ? "N/A" : name);

        // One equation per line, the same "coef*(var)+...=number" shape
        // LinearSystem::operator>> reads via getline(), with name-based
        // column placement -- `3*y=6` lands in the y column regardless of
        // how many other variables the system has.
        std::ostringstream joined;
        for (int i = 0; i < equations; ++i) {
            joined << (static_cast<size_t>(i) < equationLines.size() ? equationLines[static_cast<size_t>(i)]
                                                                       : std::string());
            joined << '\n';
        }
        std::istringstream iss(joined.str());
        iss >> system;
        Workspace::instance().add(new LinearSystem(system));
        return true;
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }
    return false;
}

bool GuiApp::createDataset(const std::string& valuesText) {
    try {
        // Splitting on commas/whitespace is plain tokenising -- a data set has
        // no coefficient/variable/exponent grammar, just a count and a list
        // of numbers. The tokens are handed to Dataset::operator>> by
        // synthesising the "choice unit count value..." stream it expects, so
        // the actual numeric conversion still goes through the same code path.
        std::vector<std::string> tokens;
        std::string current;
        for (const char c : valuesText) {
            if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) tokens.push_back(current);
        if (tokens.empty())
            throw ParseException("Enter at least one value, e.g. 1, 2, 3.5");

        std::ostringstream synthesized;
        synthesized << "n N " << tokens.size();
        for (const auto& token : tokens) synthesized << ' ' << token;

        Dataset dataset;
        std::istringstream iss(synthesized.str());
        iss >> dataset;
        Workspace::instance().add(new Dataset(dataset));
        return true;
    } catch (const MathLabException& e) {
        showError(e.getFullMessage());
    } catch (const std::exception& e) {
        showError(e.what());
    }
    return false;
}
