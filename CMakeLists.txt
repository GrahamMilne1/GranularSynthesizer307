cmake_minimum_required(VERSION 3.22)
project(GranularSynth VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)

juce_add_gui_app(GranularSynth 
    PRODUCT_NAME "GranularSynth"
)

target_sources(GranularSynth PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/grainEngine/grainEngine.cpp
    Source/grainEngine/grainEngine.h


)

target_link_libraries(GranularSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(GranularSynth PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)
