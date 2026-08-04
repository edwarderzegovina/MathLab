#ifndef MATHLAB_GUI_GUIAPP_H
#define MATHLAB_GUI_GUIAPP_H

#include <array>
#include <string>
#include <vector>

#include "panels/DatasetPanel.h"
#include "panels/EntityList.h"
#include "panels/MatrixPanel.h"
#include "panels/PolynomialPanel.h"
#include "panels/SystemPanel.h"

// Owns the whole GUI's per-frame state: the menu bar, the two-pane layout
// (entity list on the left, a type-aware inspector on the right -- a
// MatrixPanel/PolynomialPanel/SystemPanel/DatasetPanel picked by the selected
// entity's dynamic type) and the error-toast queue.
//
// Deliberately UI-only: GuiApp never computes anything about a Matrix,
// Polynomial, LinearSystem or Dataset -- it only calls Workspace methods and
// renders what they return or reports what they throw.
class GuiApp {
public:
    GuiApp();

    // Renders one frame's worth of UI. Must be called between
    // ImGui::NewFrame() and ImGui::Render(). `deltaSeconds` ages the toast
    // queue so toasts auto-dismiss.
    void renderFrame(float deltaSeconds);

    // Every catch handler in the GUI funnels here: turns a caught
    // MathLabException/std::exception message into a toast instead of
    // letting it reach main() or a message box.
    void showError(const std::string& message);

    bool wantsQuit() const { return quitRequested_; }

    // Loads/saves the whole Workspace from/to a directory, guarded so a
    // MathLabException becomes a toast.
    void loadWorkspaceDirectory(const std::string& directory);
    void saveWorkspaceDirectory(const std::string& directory);

private:
    struct Toast {
        std::string message;
        float secondsRemaining;
    };

    void renderMenuBar();
    void renderInspector();
    void renderToasts();
    void renderNewEntityPopups();
    void renderLogWindow();

    // Each backs one "New ..." modal's Create button. Every
    // MathLabException/std::exception (bad dimensions, a parse error,
    // MAX_SIZE exceeded, ...) is caught here and turned into a toast; returns
    // whether the entity was created, so the popup knows whether to close
    // itself.
    bool createMatrix(int rows, int columns);
    bool createPolynomial(const std::string& text);
    bool createLinearSystem(int variables, int equations, const std::string& name,
                             const std::vector<std::string>& equationLines);
    bool createDataset(const std::string& valuesText);

    EntityList entityList_;
    MatrixPanel matrixPanel_;
    PolynomialPanel polynomialPanel_;
    SystemPanel systemPanel_;
    DatasetPanel datasetPanel_;
    // Plain fixed-size buffers for ImGui::InputText -- no native file dialog
    // (that would be a third dependency), just a text field the user types a
    // path into.
    char loadDirectoryBuffer_[256] = "data";
    char saveDirectoryBuffer_[256] = "data";
    std::vector<Toast> toasts_;
    bool quitRequested_ = false;

    // "New ..." modal scratch state. LinearSystem's limits are asymmetric
    // (variables <= 9, equations <= 10 -- see LinearSystem.h) because the
    // augmented matrix is equations x (variables+1) and Matrix::MAX_SIZE is
    // 10; kMaxSystemEquationRows is sized to the larger of the two so every
    // legal equation count has a text field.
    static constexpr int kMaxSystemEquationRows = 10;
    int newMatrixRows_ = 2;
    int newMatrixColumns_ = 2;
    char newPolynomialBuffer_[128] = "";
    int newSystemVariables_ = 2;
    int newSystemEquations_ = 2;
    char newSystemNameBuffer_[128] = "";
    std::array<std::array<char, 128>, kMaxSystemEquationRows> newSystemEquationBuffers_{};
    char newDatasetBuffer_[256] = "";

    bool showLogWindow_ = false;
};

#endif // MATHLAB_GUI_GUIAPP_H
