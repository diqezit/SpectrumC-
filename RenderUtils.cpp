// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RenderUtils.cpp: Implementation of rendering utility functions.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "RenderUtils.h"
#include <numeric>

namespace Spectrum::RenderUtils {

    float AverageRange(const SpectrumData& spectrum, size_t begin, size_t end) {
        if (spectrum.empty()) return 0.0f;

        const size_t n = spectrum.size();
        begin = std::min(begin, n);
        end = std::min(end, n);
        if (begin >= end) return 0.0f;

        const float sum = std::accumulate(
            spectrum.begin() + begin,
            spectrum.begin() + end,
            0.0f
        );
        return sum / static_cast<float>(end - begin);
    }

    float SegmentAverage(const SpectrumData& spectrum, size_t segments, size_t index) {
        if (spectrum.empty() || segments == 0) return 0.0f;

        const size_t start = (index * spectrum.size()) / segments;
        const size_t end = ((index + 1) * spectrum.size()) / segments;
        return AverageRange(spectrum, start, end);
    }

    float GetAverageMagnitude(const SpectrumData& spectrum) {
        return AverageRange(spectrum, 0, spectrum.size());
    }

    float GetBassMagnitude(const SpectrumData& spectrum) {
        if (spectrum.empty()) return 0.0f;
        const size_t end = std::max<size_t>(1, spectrum.size() / 8);
        return AverageRange(spectrum, 0, end);
    }

    float GetMidMagnitude(const SpectrumData& spectrum) {
        if (spectrum.empty()) return 0.0f;
        const size_t start = spectrum.size() / 8;
        const size_t end = std::min(spectrum.size(), start + spectrum.size() / 2);
        return AverageRange(spectrum, start, end);
    }

    float GetHighMagnitude(const SpectrumData& spectrum) {
        if (spectrum.empty()) return 0.0f;
        const size_t start = std::min(spectrum.size(), (spectrum.size() * 5) / 8);
        return AverageRange(spectrum, start, spectrum.size());
    }

    BarLayout ComputeBarLayout(size_t count, float spacing, int viewWidth) {
        BarLayout bl{};
        bl.spacing = spacing;
        if (count == 0 || viewWidth <= 0) return bl;

        bl.totalBarWidth = static_cast<float>(viewWidth) / static_cast<float>(count);
        bl.barWidth = bl.totalBarWidth - spacing;
        if (bl.barWidth < 0.0f) bl.barWidth = 0.0f;
        return bl;
    }

    void BuildPolylineFromSpectrum(
        const SpectrumData& spectrum,
        float midlineY,
        float amplitude,
        int viewWidth,
        std::vector<Point>& out
    ) {
        const size_t n = spectrum.size();
        out.resize(n);

        for (size_t i = 0; i < n; ++i) {
            out[i].x = (static_cast<float>(i) / std::max<size_t>(1, n - 1)) * viewWidth;
            out[i].y = midlineY - spectrum[i] * amplitude;
        }
    }

    float MagnitudeToHeight(float magnitude, int viewHeight, float scale) {
        const float h = magnitude * static_cast<float>(viewHeight) * scale;
        return std::clamp(h, 0.0f, static_cast<float>(viewHeight));
    }

}