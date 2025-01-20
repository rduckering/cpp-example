#include "SettingsManager.h"
#include <iostream>
#include <fstream>

#if USE_SIMD_JSON
#include "simdjson.h"
#else
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

using namespace std::filesystem;

SettingsManager::SettingsManager (juce::ValueTree& tree) : pluginTree (tree)
{
#if IS_WINDOWS
    { // std::env unsafe on windows
        char *appDataPath = nullptr;
        size_t bufferSize = 0;
        errno_t err = _dupenv_s(&appDataPath, &bufferSize, "APPDATA");

        settingsFilePath = std::filesystem::path(appDataPath) / "GT-Tuner";
        delete[] appDataPath;
    }
#endif

    if (createDirectory (settingsFilePath))
        if (createSettingsFile (settingsFile))
            updateSettingsFromFile();
}

SettingsManager::~SettingsManager()
{
    // this needs moving from the destructor and should be set when a setting is altered
    storeSettingsToFile();
}

void SettingsManager::updateSettingsFromFile()
{
    float tuningOffset = 0.0f;
    TuningInfo::TuningMode tuningMode = TuningInfo::TuningMode::STANDARD_E;
    float sensitivityLevel = 0.0f;

    if (is_regular_file (settingsFile))
    {
#if USE_SIMD_JSON
        simdjson::ondemand::parser parser;
        simdjson::padded_string json = simdjson::padded_string::load (settingsFile.string());
        simdjson::ondemand::document settings = parser.iterate (json);

        tuningOffset = static_cast<float> (settings["tuning_offset"].get_double());
        sensitivityLevel = static_cast<float> (settings["sensitivity_level"].get_double());
        tuningMode = static_cast<TuningInfo::TuningMode> (static_cast<int> (settings["tuning_mode"].get_int64()));
#else
        std::ifstream f (settingsFile);

        if (f.peek() == std::ifstream::traits_type::eof())
            return;

        json data = json::parse (f);

        std::cout << data.dump(4) << std::endl;

        if (data.find (SettingIds::tuningOffsetId) != data.end())
            tuningOffset = data.at (SettingIds::tuningOffsetId);

        if (data.find (SettingIds::sensitivityLevelId) != data.end())
            sensitivityLevel = data.at (SettingIds::sensitivityLevelId);

        if (data.find (SettingIds::tuningModeId) != data.end())
            tuningMode = data.at (SettingIds::tuningModeId);
#endif

        pluginTree.setProperty (juce::Identifier (SettingIds::tuningOffsetId), tuningOffset, nullptr);
        pluginTree.setProperty (juce::Identifier (SettingIds::sensitivityLevelId), sensitivityLevel, nullptr);
        pluginTree.setProperty (juce::Identifier (SettingIds::tuningModeId), static_cast<int> (tuningMode), nullptr);
    }
}

void SettingsManager::storeSettingsToFile() const
{
    if (! is_regular_file (settingsFile))
        return;

    const float tuningOffset = static_cast<float> (pluginTree.getProperty (juce::Identifier (SettingIds::tuningOffsetId), { 0.0f }));
    const int tuningMode = static_cast<int> (pluginTree.getProperty (juce::Identifier (SettingIds::tuningModeId), { 0 }));
    const float sensitivityLevel = static_cast<float> (pluginTree.getProperty (juce::Identifier (SettingIds::sensitivityLevelId), { 0.0f }));

    std::ofstream file (settingsFile);

    if (file.is_open())
    {
#if USE_SIMD_JSON
        file << "{" << std::endl;
        file << R"("tuning_offset":)" << tuningOffset << "," << std::endl;
        file << R"("sensitivity_level":)" << sensitivityLevel << "," << std::endl;
        file << R"("tuning_mode":)" << tuningMode << std::endl;
        file << "}" << std::endl;
#else
        json data;
        data[SettingIds::tuningOffsetId] = tuningOffset;
        data[SettingIds::sensitivityLevelId] = sensitivityLevel;
        data[SettingIds::tuningModeId] = tuningMode;
        file << data.dump (4) << std::endl;
#endif
        file.close();
        std::cout << "Settings file created successfully." << std::endl;
    }
}

bool SettingsManager::createDirectory (const std::filesystem::path& dirPath)
{
    if (! is_directory (dirPath))
    {
        std::error_code ec;

        if (! create_directory (dirPath, ec))
        {
            std::cout << "Couldn't create directory: " << ec.value() << " - " << ec.message() << std::endl;
            return false;
        }
    }

    return true;
}

bool SettingsManager::createSettingsFile (const std::filesystem::path& filePath)
{
    if (! is_regular_file (filePath))
    {
        std::ofstream file (filePath);

        if (file.is_open())
        {
            auto output = R"({ "tuning_offset": 420.0, "sensitivity_level": 0.15, "tuning_mode": 1 })";
            file << output << std::endl;
            file.close();
            std::cout << "Settings file created successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to create settings file." << std::endl;
            return false;
        }
    }

    return true;
}