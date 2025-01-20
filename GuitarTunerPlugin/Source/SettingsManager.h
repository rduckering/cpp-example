#pragma once
#include <filesystem>
#include "TuningInfo.h"

// Setting manager - an example of using mainly standard c++ to manage settings in a project and output them
// to a json file. With the addition of an external json parsing library to speed up parsing of json/improving
// handling json like objects with the project. Options USE_SIMD_JSON in cmake to change between the two.

// this could also be put in to JUCE xml settings file that is output to applicationSupport, but just an example of doing it without JUCE.

// file handling only intended for Mac and Windows

namespace SettingIds
{
    static const std::string tuningOffsetId = "tuning_offset";
    static const std::string sensitivityLevelId = "sensitivity_level";
    static const std::string tuningModeId = "tuning_mode";
}

class SettingsManager
{
public:
    SettingsManager (juce::ValueTree& tree);
    ~SettingsManager();

    void updateSettingsFromFile();
    void storeSettingsToFile() const;

    void setTuningOffset (float value) noexcept { pluginTree.setProperty (juce::Identifier (SettingIds::tuningOffsetId), value, nullptr); }
    void setTuningMode (int value) noexcept { pluginTree.setProperty (juce::Identifier (SettingIds::tuningModeId), value, nullptr);  }
    void setSensitivityLevel (float value) noexcept { pluginTree.setProperty (juce::Identifier (SettingIds::sensitivityLevelId), value, nullptr);  }


    juce::ValueTree& getStateValueTrees() noexcept { return pluginTree; }

    [[nodiscard]] float getTuningOffset() const noexcept { return pluginTree.getProperty (juce::Identifier (SettingIds::tuningOffsetId), { 0.0f }); }
    [[nodiscard]] int getTuningMode() const noexcept { return pluginTree.getProperty (juce::Identifier (SettingIds::tuningModeId), { 0 }); }
    [[nodiscard]] float getSensitivityLevel() const noexcept { return pluginTree.getProperty (juce::Identifier (SettingIds::sensitivityLevelId), { 0.0f }); }

private:
#if IS_WINDOWS
    std::filesystem::path settingsFilePath;
#else
    const std::filesystem::path settingsFilePath = std::filesystem::path (std::getenv ("HOME")) / "Library/Application Support/GT-Tuner";
#endif

    const std::filesystem::path settingsFile = settingsFilePath / "settings.json";

    static bool createDirectory (const std::filesystem::path& dirPath);
    static bool createSettingsFile (const std::filesystem::path& filePath);

    // settings values
    juce::ValueTree& pluginTree;
};