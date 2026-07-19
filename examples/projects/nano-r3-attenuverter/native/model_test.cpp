#include "../src/attenuverter_model.hpp"

#include <cmath>
#include <cstdlib>

namespace {

bool near(const float actual, const float expected) {
    return std::fabs(actual - expected) < 0.0001F;
}

} // namespace

int main() {
    using attenuverter_example::applyCenteredDeadZone;
    using attenuverter_example::clampValue;
    using attenuverter_example::process;

    if (!near(clampValue(-2.0F, -1.0F, 1.0F), -1.0F) ||
        !near(clampValue(2.0F, -1.0F, 1.0F), 1.0F) || !near(clampValue(0.5F, -1.0F, 1.0F), 0.5F)) {
        return EXIT_FAILURE;
    }

    if (!near(applyCenteredDeadZone(0.02F, 0.035F), 0.0F)) {
        return EXIT_FAILURE;
    }

    const auto centered = process(4.0F, 0.5F, 0.0F);
    if (!near(centered.outputVolts, 2.0F)) {
        return EXIT_FAILURE;
    }

    const auto positiveFavored = process(-4.0F, 0.5F, 0.75F);
    if (!near(positiveFavored.outputVolts, -0.5F)) {
        return EXIT_FAILURE;
    }

    const auto inverted = process(-3.0F, -1.0F, -0.5F);
    if (!near(inverted.outputVolts, 1.5F)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
