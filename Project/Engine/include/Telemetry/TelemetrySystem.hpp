#pragma once
// TelemetrySystem.hpp
//
// Read-only world-state sampler for external tooling.
//
// Why this exists: several gameplay bugs on this branch were "fixed" from
// code reading alone because nobody could observe a running game. Screenshots
// are not enough - a previous debugging attempt mistook the idle animation
// for the player walking. This writes the facts (positions, health, FSM
// states) that a screenshot cannot show.
//
// It is off unless GAM300_TELEMETRY=1 is set in the environment, it never
// writes to any game state, and when disabled the per-frame cost is one
// boolean test.

#include <string>

namespace Telemetry {

    // Reads the environment and opens the output file. Safe to call twice.
    // When GAM300_TELEMETRY is not "1" this does nothing and leaves the
    // system disabled for the lifetime of the process.
    void Initialise();

    // Samples the world and appends one JSON object per line, rate-limited
    // internally. Call once per frame from the main thread, after scripts
    // have run, so the values are the ones the frame actually used.
    void Sample();

    // Flushes and closes the output file.
    void Shutdown();

    bool IsEnabled();

    // Output path, empty when disabled. Set by GAM300_TELEMETRY_PATH, and
    // defaults to telemetry.jsonl next to the executable.
    const std::string& OutputPath();

}
