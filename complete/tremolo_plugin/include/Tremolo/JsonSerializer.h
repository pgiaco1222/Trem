#pragma once

namespace tremolo {
class JsonSerializer {
public:
  static void serialize(const Parameters&, juce::OutputStream&);
  static juce::Result deserialize(juce::InputStream&, Parameters&);
};
}
