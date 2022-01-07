cmake_minimum_required(VERSION 3.14)
project(Deviant VERSION 1.0.0)

# Paths are defined here, alter as necessary... I attempted some cmake hackery to avoid having to set these manually.  I failed. 
# Other parts of this file will most likely need/want to be tailored as well.  Any feedback to improve this procedure would be appreciated.
IF (WIN32)
    set(JUCE_LOCATION "C:/Users/frye/Documents/JUCE")
    set(VST_COPY_LOCATION "C:/Program Files (x86)/Common Files/VST")
    set(VST3_COPY_LOCATION "C:/Program Files/Common Files/VST3")
    set(VST2_SDK_LOCATION "C:/Users/frye/CODE/SDKs/VST3_SDK/VST2_SDK")
    set(AUDIO_PROJECTS_LOCATION "C:/Users/frye/Music/Projects")
ELSE()
    set(JUCE_LOCATION "$ENV{HOME}/Documents/JUCE")
    set(VST_COPY_LOCATION "/Library/Audio/Plug-Ins/VST")
    set(VST3_COPY_LOCATION "/Library/Audio/Plug-Ins/VST3")
    set(VST2_SDK_LOCATION "$ENV{HOME}/CODE/SDKs/VST3_SDK/VST2_SDK")
    set(AUDIO_PROJECTS_LOCATION "$ENV{HOME}/Music/Projects/Reaper")
    set(DEBUG_APP "/Applications/REAPER64.app")
    set(DEBUG_ARGS "${AUDIO_PROJECTS_LOCATION}/DeviantTestRadiohead/DeviantTestRadiohead.RPP")
ENDIF()

# By default we don't want schemes to be made for modules, etc
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_XCODE_ATTRIBUTE_OTHER_LDFLAGS "-Xlinker -headerpad -Xlinker 578")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

# Adds all the module sources so they appear correctly in the IDE
# This needs to be set before JUCE is added as a sub-dir
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Targets")

# Create a /Modules directory with all the JUCE Module code
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

add_subdirectory(${JUCE_LOCATION} JUCE)
juce_set_vst2_sdk_path(${VST2_SDK_LOCATION})
juce_add_plugin(Deviant
    BUNDLE_ID "com.sadisticaudio.Deviant"
    PLUGIN_MANUFACTURER_CODE sTic
    PLUGIN_CODE iAnt
    COMPANY_NAME "Sadistic Audio"
    PRODUCT_NAME "Deviant"
    COMPANY_COPYRIGHT "copyright Sadistic Algorithms, LLC, 2020"
    COMPANY_EMAIL "support@sadisticaudio.com"
#    DOCUMENT_EXTENSIONS xml wav
    FORMATS AU VST VST3
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_COPY_DIR "/Library/Audio/Plug-Ins/Components"
    VST_COPY_DIR "${VST_COPY_LOCATION}"
    VST3_COPY_DIR "${VST3_COPY_LOCATION}"
    HARDENED_RUNTIME_ENABLED TRUE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    DESCRIPTION "Deviant Waveshaper"
    VST2_CATEGORY kPlugCategEffect
    VST3_CATEGORIES Fx Analyzer EQ Filter
    AU_MAIN_TYPE kAudioUnitType_Effect)

file(GLOB_RECURSE resources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Resources/*")

file(GLOB SourceFiles
    Source/*.h
    Source/*.cpp
    sadistic/*.h
    sadistic/*.cpp
)

target_sources(Deviant PRIVATE ${SourceFiles})
source_group(TREE ../ PREFIX "" FILES ${SourceFiles})

# Move the INTERFACE auto-created JUCE library stuff into its own folder
source_group("JUCE Library Code" REGULAR_EXPRESSION "juce_")

# This cleans up the folder organization, especially on Xcode.
# It tucks the Plugin varieties into a "Targets" folder and generate an Xcode Scheme manually
# since Xcode scheme generation is turned off to limit noise
# Not all plugin types (au, vst) available on each build type (win, macos, linux)
foreach(target Deviant_AU Deviant_VST Deviant_VST3 Deviant_All)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES 
            FOLDER "Targets"
            XCODE_GENERATE_SCHEME ON
            )
    endif()
endforeach()

foreach(target Deviant_AU Deviant_VST Deviant_VST3)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES 
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application: Benjamin Kelley (NXW2SG7JJ4)"
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "NXW2SG7JJ4"
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
            XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS --timestamp
            OSX_ARCHITECTURES "arm64;x86_64"
            XCODE_SCHEME_EXECUTABLE "${DEBUG_APP}"
            XCODE_SCHEME_ARGUMENTS "${DEBUG_ARGS}")
    endif()
endforeach()

juce_add_binary_data(BinaryData HEADER_NAME SadisticData.h NAMESPACE Data SOURCES ${resources})

#`target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correctvisible code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.
target_compile_features(Deviant PRIVATE cxx_std_20)
target_compile_definitions(Deviant PUBLIC
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_COPY_PLUGIN_AFTER_BUILD=TRUE
    JUCE_OPENGL3=1
)

target_link_libraries(Deviant PRIVATE
    BinaryData
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_product_unlocking
    juce::juce_dsp
    juce::juce_opengl
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
 )
