#pragma once
#include <cmath>

namespace Utils
{
    static int frequencyToMidi (float frequency) noexcept
    {
        constexpr float a4_receip = 1.0f / 440.0f;
        const float note = 69.0f + 12.0f * std::log2 (frequency * a4_receip);
        return static_cast<int> (note + 0.5f);
    }

    static float midiToFrequency (int midi) noexcept
    {
        constexpr float receip = 1.0f / 12.0f;
        const float frequency = 440.0f * (std::powf (2.0f, (static_cast<float> (midi - 69) * receip)));
        return frequency;
    }

    static juce::String midiNoteToName (int midi) noexcept
    {
        const int octave = (midi - 12) / 12;
        const int noteIndex = (midi - 12) % 12;

        if (noteIndex > 12 || noteIndex < 0)
            return "error";

        std::array<std::string, 12> noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        return juce::String (noteNames.at (static_cast<size_t> (noteIndex))) + juce::String (octave + 1);
    }
}