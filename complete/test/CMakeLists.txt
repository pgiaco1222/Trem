project(TremoloCoursePluginTest)


enable_testing()
cpmaddpackage(
  NAME
    GOOGLETEST
  GITHUB_REPOSITORY
    google/googletest
  VERSION
    1.16.0
  SOURCE_DIR
    ${LIB_DIR}/googletest
  OPTIONS
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt ON"
)

cpmaddpackage(
  NAME
    WOLFSOUND_DSP_UTILS
  GITHUB_REPOSITORY
    JanWilczek/wolfsound-dsp-utils
  VERSION
    0.4.0
  SOURCE_DIR
    ${LIB_DIR}/wolfsound-dsp-utils
  OPTIONS
    "BUILD_TESTS OFF"
)

add_executable(TremoloCoursePluginTest
  source/PluginProcessorTest.cpp
  source/JsonSerializerTest.cpp
  source/TremoloTest.cpp
  source/detail/StridedQueueTest.cpp
  source/BypassTransitionSmootherTest.cpp
)
target_compile_definitions(TremoloCoursePluginTest
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JucePlugin_Manufacturer="$<TARGET_PROPERTY:TremoloCoursePlugin,JUCE_COMPANY_NAME>"
    JucePlugin_Name="$<TARGET_PROPERTY:TremoloCoursePlugin,JUCE_PLUGIN_NAME>"
    JucePlugin_VersionString="$<TARGET_PROPERTY:TremoloCoursePlugin,JUCE_VERSION>"
)
target_link_libraries(
  TremoloCoursePluginTest
  PRIVATE
    tremolo::tremolo_plugin
    GTest::gtest_main
    wolfsound::wolfsound_dsp_utils
    juce::juce_recommended_warning_flags
)
include(GoogleTest)

gtest_discover_tests(TremoloCoursePluginTest DISCOVERY_TIMEOUT 60 DISCOVERY_MODE PRE_TEST)

