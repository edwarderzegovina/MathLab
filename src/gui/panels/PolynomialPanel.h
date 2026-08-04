#ifndef MATHLAB_GUI_POLYNOMIALPANEL_H
#define MATHLAB_GUI_POLYNOMIALPANEL_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Polynomial.h"

// Right-pane panel for a selected Polynomial: the rendered polynomial, an
// editable coefficient list, evaluation at a point, derivative/integral,
// a definite integral over [a,b], and a curve plot over an adjustable
// x-range.
//
// The curve plot renders automatically as soon as a Polynomial is selected
// (matching DatasetPanel's histogram) rather than waiting for a button --
// but it only resamples the 100 points when the id, the x-range, or the
// polynomial's own contents (coefficients edited above) actually changed
// since the last frame, not on every frame; see renderPlot's cache fields
// below.
//
// Deliberately UI-only: every number shown or written comes from a Workspace
// call (Polynomial.h is included only for the type, to hold a computed
// derivative/integral result until "Add to workspace" is clicked). Any
// MathLabException/std::exception raised talking to the Workspace is
// reported through errorSink instead of propagating.
class PolynomialPanel {
public:
    void render(int id, const std::function<void(const std::string&)>& errorSink);

private:
    void renderCoefficients(int id, const std::function<void(const std::string&)>& errorSink);
    void renderEvaluation(int id, const std::function<void(const std::string&)>& errorSink);
    void renderCalculus(int id, const std::function<void(const std::string&)>& errorSink);
    void renderPlot(int id, const std::function<void(const std::string&)>& errorSink);

    int lastId_ = -1;

    float evalPoint_ = 0.0f;
    std::string evalResultText_;

    float integralUpper_ = 1.0f;
    float integralLower_ = 0.0f;
    std::string definiteIntegralText_;

    float plotXMin_ = -10.0f;
    float plotXMax_ = 10.0f;
    std::vector<float> plotSamples_;
    bool plotValid_ = false;

    // Cache-invalidation fingerprint for the auto-plot: the id/range/content
    // the samples above were last computed for. Recomputed only when one of
    // these differs from the current frame's values, so an unchanged curve
    // isn't resampled at 100 points every single frame.
    int plotCachedId_ = -1;
    float plotCachedXMin_ = 0.0f;
    float plotCachedXMax_ = 0.0f;
    std::string plotCachedContent_;

    // The most recently computed derivative/integral, kept alive so
    // "Add to workspace" can commit it without recomputing.
    std::string resultLabel_;
    std::string resultText_;
    std::unique_ptr<Polynomial> resultPolynomial_;
};

#endif // MATHLAB_GUI_POLYNOMIALPANEL_H
