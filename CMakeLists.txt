cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

if(SMTG_MAC)
    set(vst3sdk_SOURCE_DIR "/Users/yoon/VST_SDK/vst3sdk")
elseif(SMTG_WIN)
    set(vst3sdk_SOURCE_DIR "C:/VST_SDK/vst3sdk")
endif(SMTG_MAC)

if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(SVEQ
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "SVEQ VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(SVEQ
    source/version.h
    source/SVEQ_cids.h
    source/SVEQ_processor.h
    source/SVEQ_processor.cpp
    source/SVEQ_controller.h
    source/SVEQ_controller.cpp
    source/SVEQ_entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(SVEQ
        PRIVATE
            resource/SVEQ_editor.uidesc

            resource/knob_blue.png
            resource/knob_blue_2.0x.png
            resource/knob_blue_sml.png
            resource/knob_blue_sml_2.0x.png

            resource/knob_black.png
            resource/knob_black_2.0x.png
            resource/knob_black_sml.png
            resource/knob_black_sml_2.0x.png

            resource/knob_green.png
            resource/knob_green_2.0x.png
            resource/knob_green_sml.png
            resource/knob_green_sml_2.0x.png

            resource/knob_red.png
            resource/knob_red_2.0x.png
            resource/knob_red_sml.png
            resource/knob_red_sml_2.0x.png

            resource/knob_purple.png
            resource/knob_purple_2.0x.png
            resource/knob_purple_sml.png
            resource/knob_purple_sml_2.0x.png
            
            resource/knob_yellow.png
            resource/knob_yellow_2.0x.png
            resource/knob_yellow_sml.png
            resource/knob_yellow_sml_2.0x.png

            resource/btn_curve.png
            resource/btn_curve_2.0x.png

            resource/PretendardMono-Regular.ttf
    )
    target_link_libraries(SVEQ
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(SVEQ
        RESOURCES
            "resource/SVEQ_editor.uidesc"
            
            "resource/knob_blue.png"
            "resource/knob_blue_2.0x.png"
            "resource/knob_blue_sml.png"
            "resource/knob_blue_sml_2.0x.png"

            "resource/knob_black.png"
            "resource/knob_black_2.0x.png"
            "resource/knob_black_sml.png"
            "resource/knob_black_sml_2.0x.png"

            "resource/knob_green.png"
            "resource/knob_green_2.0x.png"
            "resource/knob_green_sml.png"
            "resource/knob_green_sml_2.0x.png"

            "resource/knob_red.png"
            "resource/knob_red_2.0x.png"
            "resource/knob_red_sml.png"
            "resource/knob_red_sml_2.0x.png"

            "resource/knob_purple.png"
            "resource/knob_purple_2.0x.png"
            "resource/knob_purple_sml.png"
            "resource/knob_purple_sml_2.0x.png"
            
            "resource/knob_yellow.png"
            "resource/knob_yellow_2.0x.png"
            "resource/knob_yellow_sml.png"
            "resource/knob_yellow_sml_2.0x.png"

            "resource/btn_curve.png"
            "resource/btn_curve_2.0x.png"
    )
    smtg_target_add_plugin_resources (SVEQ
     RESOURCES
       "resource/PretendardMono-Regular.ttf"
     OUTPUT_SUBDIRECTORY
       Fonts
   )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots (SVEQ
    RESOURCES
        resource/FD13F19541CF5F22AC337E9F2FF7054F_snapshot.png
        resource/FD13F19541CF5F22AC337E9F2FF7054F_snapshot_2.0x.png
)

target_link_libraries(SVEQ
    PRIVATE
        sdk
)

set(SIMDE_PATH "${PROJECT_BINARY_DIR}/../libs/simde" CACHE STRING "Path to simde library source tree")
add_library(simde INTERFACE)
target_include_directories(simde INTERFACE ${SIMDE_PATH})
target_link_libraries(SVEQ
    PRIVATE 
        simde
)
smtg_target_setup_universal_binary(simde)

smtg_target_configure_version_file(SVEQ)

if(SMTG_MAC)
    smtg_target_set_bundle(SVEQ
        BUNDLE_IDENTIFIER io.github.yg331.SVEQ
        COMPANY_NAME "yg331"
    )
    smtg_target_set_debug_executable(SVEQ
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(SVEQ PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SVEQ)

        smtg_target_set_debug_executable(SVEQ
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)

# Add an AUv2 target
if (SMTG_MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
	smtg_target_add_auv2(SVEQ-au
		BUNDLE_NAME SVEQ
		BUNDLE_IDENTIFIER io.githuib.yg331.SVEQ.audiounit
		INFO_PLIST_TEMPLATE resource/au-info.plist
		VST3_PLUGIN_TARGET SVEQ)
endif(SMTG_MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
