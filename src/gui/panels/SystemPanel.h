#ifndef MATHLAB_GUI_SYSTEMPANEL_H
#define MATHLAB_GUI_SYSTEMPANEL_H

#include <functional>
#include <memory>
#include <string>

#include "LinearSystem.h"
#include "SolutionReport.h"

// Right-pane panel for a selected LinearSystem: the equations, an
// automatically-solved SolutionReport rendered structurally (a
// variable/value table for a unique solution, the parametrised expressions
// and free parameters for infinitely many), free-term display, and a
// binary-op row against another system in the workspace.
//
// The solve runs as soon as a LinearSystem is selected (matching
// DatasetPanel's histogram and PolynomialPanel's curve plot) rather than
// waiting for a button -- but only when the id or the system's contents
// changed since the last frame, not on every frame; see renderSolve's cache
// fields below. Gauss elimination + Rouche-Capelli classification is cheap
// for the sizes this project caps out at (equations <= 10), but "cheap once"
// is not the same as "free at 60 frames a second forever".
//
// An inconsistent system makes LinearSystem::solveSystem() throw a
// CalculationException (see SolutionReport.h) instead of returning a report
// with SolutionKind::Inconsistent -- that is a documented, deliberate
// behaviour, not a bug, so this panel catches it and renders "No solution"
// as a normal part of the panel rather than routing it through the error
// toast queue.
//
// Deliberately UI-only: every number shown comes from a Workspace call.
class SystemPanel {
public:
    void render(int id, const std::function<void(const std::string&)>& errorSink);

private:
    void renderEquations(int id, const std::function<void(const std::string&)>& errorSink);
    void renderFreeTerms(int id, const std::function<void(const std::string&)>& errorSink);
    // `content` is the system's already-fetched toString() for this frame
    // (render() reads it anyway, to display the equations), reused here as
    // the cache-invalidation fingerprint instead of fetching it a second
    // time.
    void renderSolve(int id, const std::string& content,
                      const std::function<void(const std::string&)>& errorSink);
    void renderBinaryOp(int id, const std::function<void(const std::string&)>& errorSink);

    int lastId_ = -1;

    bool hasSolved_ = false;
    bool inconsistent_ = false;
    std::string inconsistentMessage_;
    SolutionReport solveReport_;

    // Cache-invalidation fingerprint for the auto-solve: the id/content the
    // report above was last solved for.
    int solveCachedId_ = -1;
    std::string solveCachedContent_;

    int otherId_ = -1;
    int opIndex_ = 0;   // 0='+', 1='-'
    std::string resultLabel_;
    std::string resultText_;
    std::unique_ptr<LinearSystem> resultSystem_;
};

#endif // MATHLAB_GUI_SYSTEMPANEL_H
