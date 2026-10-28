#include "EditorAppearance.h"
#include <cassert>
#include <cmath>
int main() {
  assert(EditorAppearance_marking(kEditorAppearancePalette::ORIGINAL) == nullptr);
  auto a = EditorAppearance_marking(kEditorAppearancePalette::MIST_BLUE_GREY);
  auto b = EditorAppearance_marking(kEditorAppearancePalette::CLAY_GREY);
  assert(a && b);
  assert(std::abs(a->spectrogramDarkest.red - 113.0/255.0) < 1e-8);
  assert(std::abs(b->spectrogramDarkest.red - 159.0/255.0) < 1e-8);
  assert(EditorAppearance_colourMapForDrawing(kSpectrogram_colourMap::HOT, kEditorAppearancePalette::ORIGINAL) == kSpectrogram_colourMap::HOT);
  assert(EditorAppearance_colourMapForDrawing(kSpectrogram_colourMap::HOT, kEditorAppearancePalette::MIST_BLUE_GREY) == kSpectrogram_colourMap::MIST_BLUE_GREY);
  assert(std::abs(EditorAppearance_lineWidth(192, 1.0) - 2.0) < 1e-8);
  assert(std::abs(EditorAppearance_diameter(96) - 25.4) < 1e-8);
}
int wingwmain(int, char**) { return 0; }
