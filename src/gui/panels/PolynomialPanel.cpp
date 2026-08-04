#include "PolynomialPanel.h"

#include <cfloat>
#include <sstream>

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

constexpr int kPlotSampleCount = 100;

} // namespace

void PolynomialPanel::render(int id, const std::function<void(const std::string&)>& errorSink) {
    if (id != lastId_) {
        lastId_ = id;
        evalResultText_.clear();
        definiteIntegralText_.clear();
        resultLabel_.clear();
        resultText_.clear();
        resultPolynomial_.reset();
        plotValid_ = false;
    }

    std::string rendered;
    std::string name;
    bool found = false;
    guarded(errorSink, [&] {
        const Polynomial* current = Workspace::instance().polynomialById(id);
        rendered = current->toString();
        name = current->getName();
        found = true;
    });
    if (!found) {
        ImGui::TextDisabled("Polynomial is no longer available.");
        return;
    }

    ImGui::Text("Polynomial #%d \"%s\"", id, name.c_str());
    ImGui::TextWrapped("%s", rendered.c_str());
    ImGui::Separator();

    renderCoefficients(id, errorSink);
    ImGui::Separator();
    renderEvaluation(id, errorSink);
    ImGui::Separator();
    renderCalculus(id, errorSink);
    ImGui::Separator();
    renderPlot(id, errorSink);
}

void PolynomialPanel::renderCoefficients(int id,
                                          const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Coefficients (editable)");

    size_t maxDegree = 0;
    bool found = false;
    guarded(errorSink, [&] {
        maxDegree = Workspace::instance().polynomialMaxDegree(id);
        found = true;
    });
    if (!found) return;

    for (size_t power = 0; power <= maxDegree; ++power) {
        float value = 0.0f;
        guarded(errorSink,
                [&] { value = Workspace::instance().polynomialCoefficient(id, power); });

        ImGui::PushID(static_cast<int>(power));
        ImGui::SetNextItemWidth(100.0f);
        const std::string label = "x^" + std::to_string(power);
        if (ImGui::InputFloat(label.c_str(), &value)) {
            guarded(errorSink, [&] {
                Workspace::instance().polynomialSetCoefficient(id, power, value);
            });
        }
        ImGui::PopID();
    }
}

void PolynomialPanel::renderEvaluation(int id,
                                        const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Evaluate at a point");
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputFloat("x##evalPoint", &evalPoint_);
    ImGui::SameLine();
    if (ImGui::Button("Evaluate")) {
        guarded(errorSink, [&] {
            const float value = Workspace::instance().polynomialAt(id, evalPoint_);
            std::ostringstream os;
            os << "P(" << evalPoint_ << ") = " << value;
            evalResultText_ = os.str();
        });
    }
    if (!evalResultText_.empty()) ImGui::TextUnformatted(evalResultText_.c_str());
}

void PolynomialPanel::renderCalculus(int id,
                                      const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Calculus");

    if (ImGui::Button("Derivative")) {
        guarded(errorSink, [&] {
            Polynomial derivative = Workspace::instance().polynomialDerivative(id);
            resultLabel_ = "Derivative";
            resultText_ = derivative.toString();
            resultPolynomial_ = std::make_unique<Polynomial>(derivative);
        });
    }
    ImGui::SameLine();
    if (ImGui::Button("Integral")) {
        guarded(errorSink, [&] {
            Polynomial integral = Workspace::instance().polynomialIntegral(id);
            resultLabel_ = "Integral";
            resultText_ = integral.toString();
            resultPolynomial_ = std::make_unique<Polynomial>(integral);
        });
    }
    if (!resultText_.empty()) {
        ImGui::TextUnformatted(resultLabel_.c_str());
        ImGui::TextWrapped("%s", resultText_.c_str());
        if (resultPolynomial_) {
            if (ImGui::Button("Add result to workspace")) {
                guarded(errorSink, [&] {
                    Workspace::instance().add(new Polynomial(*resultPolynomial_));
                });
                resultPolynomial_.reset();
                resultText_.clear();
            }
        }
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Definite integral over [lower, upper]");
    // polynomialDefiniteIntegral(id, upper, lower) takes UPPER first -- kept
    // exactly as Workspace/Polynomial define it; the labels below make the
    // argument order visible instead of silently swapping it.
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("Lower bound", &integralLower_);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("Upper bound", &integralUpper_);
    if (ImGui::Button("Compute definite integral")) {
        guarded(errorSink, [&] {
            const float value = Workspace::instance().polynomialDefiniteIntegral(
                id, integralUpper_, integralLower_);
            std::ostringstream os;
            os << "Integral from " << integralLower_ << " to " << integralUpper_ << " = " << value;
            definiteIntegralText_ = os.str();
        });
    }
    if (!definiteIntegralText_.empty()) ImGui::TextUnformatted(definiteIntegralText_.c_str());
}

void PolynomialPanel::renderPlot(int id, const std::function<void(const std::string&)>& errorSink) {
    ImGui::TextUnformatted("Curve plot");
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("x min", &plotXMin_);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputFloat("x max", &plotXMax_);

    // Renders automatically -- no click required, matching DatasetPanel's
    // histogram -- but the 100-point resample only runs when something the
    // curve actually depends on changed: the selected id, the x-range, or
    // the polynomial's own rendered content (coefficients may have just been
    // edited above, in this same frame, by renderCoefficients). Fetching
    // toString() here is one cheap string build; it happens every frame
    // regardless, same as the "Polynomial #.. rendered" text above already
    // does -- what's guarded is the O(100) polynomialAt() loop.
    std::string content;
    guarded(errorSink,
            [&] { content = Workspace::instance().polynomialById(id)->toString(); });
    const bool dirty = (id != plotCachedId_) || (plotXMin_ != plotCachedXMin_) ||
                        (plotXMax_ != plotCachedXMax_) || (content != plotCachedContent_);
    if (dirty) {
        plotSamples_.assign(kPlotSampleCount, 0.0f);
        plotValid_ = false;
        guarded(errorSink, [&] {
            for (int i = 0; i < kPlotSampleCount; ++i) {
                const float t = static_cast<float>(i) / static_cast<float>(kPlotSampleCount - 1);
                const float x = plotXMin_ + t * (plotXMax_ - plotXMin_);
                plotSamples_[static_cast<size_t>(i)] = Workspace::instance().polynomialAt(id, x);
            }
            plotValid_ = true;
        });
        plotCachedId_ = id;
        plotCachedXMin_ = plotXMin_;
        plotCachedXMax_ = plotXMax_;
        plotCachedContent_ = content;
    }

    if (plotValid_ && !plotSamples_.empty()) {
        ImGui::PlotLines("P(x)", plotSamples_.data(), static_cast<int>(plotSamples_.size()),
                          0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, 120));
    } else if (!plotValid_) {
        ImGui::TextDisabled("(plot unavailable)");
    }
}
