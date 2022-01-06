cmake_minimum_required(VERSION 3.14)
project(Deviant VERSION 1.0.0)

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

get_filename_component(CODE ../../ ABSOLUTE)
#include_directories(${CODE}/SDKs/VST3_SDK/VST2_SDK)

IF (WIN32)
    set(AAX_COPY_LOCATION "C:/Program Files/Common Files/Avid/Audio/Plug-Ins")
    set(VST_COPY_LOCATION "C:/Program Files (x86)/Common Files/VST")
    set(VST3_COPY_LOCATION "C:/Program Files/Common Files/VST3")
    set(AUDIO_PROJECTS_LOCATION "C:/Users/frye/Music/Projects")

ELSE()
    set(AAX_COPY_LOCATION "/Library/Application Support/Avid/Audio/Plug-Ins")
    set(VST_COPY_LOCATION "/Library/Audio/Plug-Ins/VST")
    set(VST3_COPY_LOCATION "/Library/Audio/Plug-Ins/VST3")
    set(AUDIO_PROJECTS_LOCATION "/Users/omar/Music/Projects/Reaper")
ENDIF()

add_subdirectory("$ENV{HOME}/Documents/JUCE" JUCE)
juce_set_vst2_sdk_path($ENV{HOME}/CODE/SDKs/VST3_SDK/VST2_SDK)
juce_set_aax_sdk_path($ENV{HOME}/CODE/SDKs/AAX_SDK)
juce_add_plugin(Deviant
    BUNDLE_ID "com.sadisticaudio.Deviant"
    PLUGIN_MANUFACTURER_CODE sTic
    PLUGIN_CODE iAnt
    COMPANY_NAME "Sadistic Audio"
    PRODUCT_NAME "Deviant"
    COMPANY_COPYRIGHT "copyright Sadistic Algorithms, LLC, 2020"
    COMPANY_EMAIL "support@sadisticaudio.com"
    DOCUMENT_EXTENSIONS xml wav
    FORMATS AU VST VST3 AAX
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_COPY_DIR "/Library/Audio/Plug-Ins/Components"
    AAX_COPY_DIR "${AAX_COPY_LOCATION}"
    VST_COPY_DIR "${VST_COPY_LOCATION}"
    VST3_COPY_DIR "${VST3_COPY_LOCATION}"
    HARDENED_RUNTIME_ENABLED TRUE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    DESCRIPTION "Deviant Sidechain EQ"
    VST2_CATEGORY kPlugCategEffect
    VST3_CATEGORIES Fx Analyzer EQ Filter
    AU_MAIN_TYPE kAudioUnitType_Effect
    AAX_CATEGORY AAX_ePlugInCategory_EQ)

file(GLOB_RECURSE resources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Resources/*")
file(GLOB_RECURSE bundle_resources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Resources/Deviant.xml")

file(GLOB SourceFiles
    Source/*.h
    Source/*.cpp
    sadistic/*.h
    sadistic/*.cpp)

target_sources(Deviant PRIVATE ${SourceFiles})
source_group(TREE ../ PREFIX "" FILES ${SourceFiles})
# Move the INTERFACE auto-created JUCE library stuff into its own folder
source_group("JUCE Library Code" REGULAR_EXPRESSION "juce_")
# This cleans up the folder organization, especially on Xcode.
# It tucks the Plugin varieties into a "Targets" folder and generate an Xcode Scheme manually
# since Xcode scheme generation is turned off to limit noise
# Not all plugin types (au, vst) available on each build type (win, macos, linux)
foreach(target Deviant_AAX Deviant_AU Deviant_VST Deviant_VST3 Deviant_All)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES 
            FOLDER "Targets"
            XCODE_GENERATE_SCHEME ON
            )
    endif()
endforeach()

#set(AAX_BUNDLE "/Library/Application Support/Avid/Audio/Plug-Ins/Deviant.aaxplugin")

foreach(target Deviant_AAX)
    if(TARGET ${target})
        set_target_properties(${target} PROPERTIES 
            OSX_ARCHITECTURES "x86_64"
            XCODE_SCHEME_EXECUTABLE "/Applications/Pro Tools Developer.app")
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
            XCODE_SCHEME_EXECUTABLE "/Applications/REAPER64.app"
            XCODE_SCHEME_ARGUMENTS "${AUDIO_PROJECTS_LOCATION}/DeviantTestRadiohead/DeviantTestRadiohead.RPP")
    endif()
endforeach()

add_custom_command(
TARGET Deviant_AAX POST_BUILD COMMAND 
COMMAND rm -f ~/Library/Preferences/Avid/Pro\ Tools/InstalledAAXPlugIns
)

juce_add_binary_data(BinaryData HEADER_NAME SadisticData.h NAMESPACE Data SOURCES ${resources})

# juce_add_bundle_resources_directory(Deviant_AAX Resources)

add_library(ALL_IPP INTERFACE)
IF (WIN32)
	set(IPP_ROOT "C:/Program Files (x86)/Intel/oneAPI/ipp/2021.1.1")
	set (IPP_LIB "${IPP_ROOT}/lib/intel64")
ELSE()
	set(IPP_ROOT "/opt/intel/oneapi/ipp/2021.1.1")
	set (IPP_LIB "${IPP_ROOT}/lib")
ENDIF()

target_include_directories(ALL_IPP INTERFACE "${IPP_ROOT}/include")

add_library(ippi    STATIC IMPORTED GLOBAL)
add_library(ipps    STATIC IMPORTED GLOBAL)
add_library(ippvm   STATIC IMPORTED GLOBAL)
add_library(ippcore STATIC IMPORTED GLOBAL)

IF (WIN32)
	set_target_properties(ippi    PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/ippi.lib)
	set_target_properties(ipps    PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/ipps.lib)
	set_target_properties(ippvm   PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/ippvm.lib)
	set_target_properties(ippcore PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/ippcore.lib)
ELSE()
	set_target_properties(ippi    PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/libippi.a    OSX_ARCHITECTURES x86_64)
	set_target_properties(ipps    PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/libipps.a    OSX_ARCHITECTURES x86_64)
	set_target_properties(ippvm   PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/libippvm.a   OSX_ARCHITECTURES x86_64)
	set_target_properties(ippcore PROPERTIES IMPORTED_LOCATION ${IPP_LIB}/libippcore.a OSX_ARCHITECTURES x86_64)
ENDIF()

target_sources(Deviant_AAX PRIVATE ${bundle_resources})
set_source_files_properties(${bundle_resources} PROPERTIES
  MACOSX_PACKAGE_LOCATION "Resources"
)
source_group("Resources" FILES ${bundle_resources})

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
    JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS=1
    )

target_link_libraries(ALL_IPP INTERFACE ippi ipps ippvm ippcore)

target_link_libraries(Deviant PRIVATE
    BinaryData
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_product_unlocking
    juce::juce_dsp
    juce::juce_opengl
    ALL_IPP
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
 )
