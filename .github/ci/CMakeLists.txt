# CI(GitHub Actions) 전용 빌드 파일입니다.
# wxCrusher.jucer 와 같은 플러그인을 CMake 로 빌드합니다. 평소 개발은 Projucer + .jucer 를 쓰면 됩니다.

cmake_minimum_required(VERSION 3.22)

project(wxCrusher VERSION 1.0.0)

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(JUCE_DIR "${REPO_ROOT}/JUCE" CACHE PATH "JUCE source folder")

add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_binary_data(wxCrusherFonts
    SOURCES
        "${REPO_ROOT}/Source/Fonts/Anton-Regular.ttf"
        "${REPO_ROOT}/Source/Fonts/SpaceMono-Bold.ttf")

# 코드는 wxCrusher.jucer 와 같아야 DAW 가 같은 플러그인으로 인식합니다.
juce_add_plugin(wxCrusher
    COMPANY_NAME "wxxmin"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Hbtb
    FORMATS VST3
    PRODUCT_NAME "wxCrusher")

juce_generate_juce_header(wxCrusher)

file(GLOB WX_SOURCES CONFIGURE_DEPENDS "${REPO_ROOT}/Source/*.cpp")
target_sources(wxCrusher PRIVATE ${WX_SOURCES})

target_compile_definitions(wxCrusher
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(wxCrusher
    PRIVATE
        wxCrusherFonts
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
