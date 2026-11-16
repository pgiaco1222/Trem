#include <tremolo_plugin/tremolo_plugin.h>
#include <gtest/gtest.h>

namespace tremolo {
class BypassTransitionSmootherTest : public testing::Test {
protected:
  static constexpr auto dryValue = 0;
  static constexpr auto wetValue = 10;

  void SetUp() override {
    constexpr auto sampleRate = 10;
    constexpr auto blockSize = sampleRate;
    constexpr auto channelCount = 1;
    testee.prepare({sampleRate, blockSize, channelCount});
    testee.setBypassForced(false);
    buffer.setSize(channelCount, blockSize);
  }

  void processTransitionBlock() {
    auto block = getBlock();
    block.fill(dryValue);
    testee.setDryBuffer(buffer);
    block.fill(wetValue);
    testee.mixToWetBuffer(buffer);
  }

  BypassTransitionSmoother testee{1.0};
  juce::AudioBuffer<float> buffer;

private:
  juce::dsp::AudioBlock<float> getBlock() { return buffer; }
};

TEST_F(BypassTransitionSmootherTest, OffOnTransitionIsSmooth) {
  testee.setBypass(true);
  ASSERT_TRUE(testee.isTransitioning());

  processTransitionBlock();

  EXPECT_FALSE(testee.isTransitioning());
  for (const auto i : std::views::iota(dryValue, wetValue)) {
    const auto expectedSample = wetValue - i - 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i), 0.0001f);
  }
}

TEST_F(BypassTransitionSmootherTest, OnOffTransitionIsSmooth) {
  testee.setBypassForced(true);
  ASSERT_FALSE(testee.isTransitioning());

  testee.setBypass(false);
  ASSERT_TRUE(testee.isTransitioning());

  processTransitionBlock();

  EXPECT_FALSE(testee.isTransitioning());
  for (const auto i : std::views::iota(dryValue, wetValue)) {
    const auto expectedSample = i + 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i), 0.0001f);
  }
}

TEST_F(BypassTransitionSmootherTest,
       OffOnTransitionIsContinuedThroughoutBlocks) {
  testee.setBypass(true);
  ASSERT_TRUE(testee.isTransitioning());

  buffer.setSize(buffer.getNumChannels(), buffer.getNumSamples() / 2);

  processTransitionBlock();
  EXPECT_TRUE(testee.isTransitioning());
  for (const auto i : std::views::iota(dryValue, wetValue / 2)) {
    const auto expectedSample = wetValue - i - 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i), 0.0001f);
  }

  processTransitionBlock();
  EXPECT_FALSE(testee.isTransitioning());
  for (const auto i : std::views::iota(wetValue / 2, wetValue)) {
    const auto expectedSample = wetValue - i - 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i - wetValue / 2), 0.0001f);
  }
}

TEST_F(BypassTransitionSmootherTest, TogglingBypassMidOffOnTransitionIsSmooth) {
  testee.setBypass(true);
  ASSERT_TRUE(testee.isTransitioning());

  buffer.setSize(buffer.getNumChannels(), buffer.getNumSamples() / 2);

  processTransitionBlock();

  EXPECT_TRUE(testee.isTransitioning());
  for (const auto i : std::views::iota(dryValue, wetValue / 2)) {
    const auto expectedSample = wetValue - i - 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i), 0.0001f);
  }

  testee.setBypass(false);
  EXPECT_TRUE(testee.isTransitioning());

  processTransitionBlock();

  EXPECT_FALSE(testee.isTransitioning());
  for (const auto i : std::views::iota(wetValue / 2, wetValue)) {
    const auto expectedSample = i + 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i - wetValue / 2), 0.0001f);
  }
}
TEST_F(BypassTransitionSmootherTest, TogglingBypassMidOnOffTransitionIsSmooth) {
  testee.setBypassForced(true);
  ASSERT_FALSE(testee.isTransitioning());

  testee.setBypass(false);
  buffer.setSize(buffer.getNumChannels(), buffer.getNumSamples() / 2);
  processTransitionBlock();

  EXPECT_TRUE(testee.isTransitioning());
  for (const auto i : std::views::iota(dryValue, wetValue / 2)) {
    const auto expectedSample = i + 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i), 0.0001f);
  }

  testee.setBypass(true);
  EXPECT_TRUE(testee.isTransitioning());

  processTransitionBlock();

  EXPECT_FALSE(testee.isTransitioning());
  for (const auto i : std::views::iota(dryValue, wetValue / 2)) {
    const auto expectedSample = wetValue / 2 - i - 1;
    EXPECT_NEAR(expectedSample, buffer.getSample(0, i), 0.0001f);
  }
}

TEST_F(BypassTransitionSmootherTest, ForcingBypassOnSkipsTransition) {
  testee.setBypassForced(true);
  EXPECT_FALSE(testee.isTransitioning());
  processTransitionBlock();
  EXPECT_FALSE(testee.isTransitioning());
  for (const auto sample :
       std::span{buffer.getReadPointer(0),
                 static_cast<size_t>(buffer.getNumSamples())}) {
    EXPECT_FLOAT_EQ(dryValue, sample);
  }
}

TEST_F(BypassTransitionSmootherTest, ForcingBypassOffSkipsTransition) {
  testee.setBypass(true);
  processTransitionBlock();
  ASSERT_FALSE(testee.isTransitioning());

  testee.setBypassForced(false);
  EXPECT_FALSE(testee.isTransitioning());
  processTransitionBlock();
  EXPECT_FALSE(testee.isTransitioning());
  for (const auto sample :
       std::span{buffer.getReadPointer(0),
                 static_cast<size_t>(buffer.getNumSamples())}) {
    EXPECT_FLOAT_EQ(wetValue, sample);
  }
}
}
