
add_subdirectory(Submodules/JUCE)
add_subdirectory(Fonts)

if(WIN32)
	set(IS_WINDOWS True)
else(APPLE)
	set(IS_WINDOWS False)
endif()

# Add option for SIMD JSON else use nlohmann JSON
option(USE_SIMD_JSON "Use simd JSON library" OFF)

juce_add_plugin(${PROJECT}
    VERSION 0.0.1                           
    COMPANY_NAME ReubenDuckering
    IS_SYNTH FALSE                  
    NEEDS_MIDI_INPUT FALSE           
    NEEDS_MIDI_OUTPUT FALSE             
    IS_MIDI_EFFECT FALSE               
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE    
    COPY_PLUGIN_AFTER_BUILD TRUE       
    PLUGIN_MANUFACTURER_CODE Juce               
    PLUGIN_CODE Dem0                   
    FORMATS AU Standalone VST3
    PRODUCT_NAME "GuitarTunerPlugin")

set(JUCE_DONT_DECLARE_PROJECTINFO True)
juce_generate_juce_header(${PROJECT})

file(GLOB SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

target_sources(${PROJECT}
    PRIVATE
		${SOURCE_FILES})

if(APPLE)
	target_compile_options(${PROJECT} PUBLIC -Wno-shadow-field)
endif()

target_compile_definitions(${PROJECT}
    PUBLIC
        JUCE_WEB_BROWSER=0  
        JUCE_USE_CURL=0     
        JUCE_VST3_CAN_REPLACE_VST2=0)

if (IS_WINDOWS)
	find_package(IPP)
	if(IPP_FOUND)
		target_link_libraries(${PROJECT} PRIVATE IPP::ippcore IPP::ipps IPP::ippi IPP::ippcv)
		set(WITH_IPP 1)
	else ()
		message(WARNING "IPP not found, using default FFT")
		set(WITH_IPP 0)
	endif()

	target_compile_definitions(${PROJECT}
		PUBLIC
			IS_WINDOWS=1
			WITH_IPP=${WITH_IPP})
endif ()

if (USE_SIMD_JSON)
	target_compile_definitions(${PROJECT}
		PUBLIC
			USE_SIMD_JSON=1)
endif ()

target_link_libraries(${PROJECT}
    PRIVATE
		MyBinaryData
        juce::juce_audio_utils
		juce::juce_audio_basics
		juce::juce_audio_formats
		juce::juce_audio_plugin_client
		juce::juce_audio_processors
		juce::juce_core
		juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

include(FetchContent)

if(USE_SIMD_JSON)
	FetchContent_Declare(
			simdjson
			GIT_REPOSITORY https://github.com/simdjson/simdjson.git
			GIT_SHALLOW TRUE
			GIT_TAG tags/v1.0.0
	)

	FetchContent_MakeAvailable(simdjson)

	target_link_libraries(${PROJECT} PRIVATE simdjson)
else()
	FetchContent_Declare(
			nlohmannjson
			URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

	FetchContent_MakeAvailable(nlohmannjson)

	target_link_libraries(${PROJECT} PRIVATE nlohmann_json::nlohmann_json)
endif()
