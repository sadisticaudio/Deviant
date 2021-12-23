#pragma once
#include "deviant.h"
namespace sadistic {
    
    enum { gain, phase, numModes };
    
    struct GUITable {
        GUITable(float* g, float*) : table { g, float(SCOPESIZE) } {}
        float operator[](float norm) const { return table[norm]; }
        Table<float> table;
        const String type { "static" }, xString { "A M P L I T U D E   I N" }, yString { "A M P L I T U D E   O U T" }, buttonString { "S T A T I C" };
        const Colour textColour { Colours::black }, bgColour { Colours::grey.darker() };
    };
    
    struct DualGUITable {
        DualGUITable(float* g, float* p) : gainTable { g, float(SCOPESIZE), 0.5f, 0.5f }, phaseTable { p, float(WAVELENGTH) } {}
        float operator[](float sample) const {
            float floatIndex { jlimit(0.f, 1.f, sample) * float(WAVELENGTH-1) };
            auto i { truncatePositiveToUnsignedInt (floatIndex) };
            float f { floatIndex - float (i) };
            jassert (isPositiveAndNotGreaterThan (f, 1.f));
            float x0 { phaseTable.table[i] }, x1 { phaseTable.table[i + 1] };
//            float x0 { phaseTable[static_cast<int>(i)] }, x1 { phaseTable[static_cast<int>(i + 1)] };
            return gainTable[jmap (f, x0, x1)]; }
//            return gainTable[roundToInt(jlimit(0.f, 1.f, sample) * float(WAVELENGTH-1))]; }
        Table<float> gainTable, phaseTable;
        const String type { "dynamic" }, xString { "P   H   A   S   E" }, yString { "A M P L I T U D E" }, buttonString { "D Y N A M I C" };
        const Colour textColour { Colours::white }, bgColour { Colours::blue.darker() };
    };
    
    template <typename TableType> struct WavePad : public Component {
        static constexpr int scopeSize { SCOPESIZE };
        WavePad(float* g, float* p) : table(g, p) {
            setInterceptsMouseClicks(true, true);
            wavePath.preallocateSpace(scopeSize * 4); }
        float gainToY(float gain) { auto h { float(getHeight()) }; return h/2 - gain * (h/2) * 0.707f; }
        float indexToX(int i) { return 15.f + (float(i)/float(scopeSize)) * (getLocalBounds().toFloat().getWidth() - 25.f); }
        void paint (Graphics& g) override {
            wavePath.clear();
            wavePath.startNewSubPath(indexToX(0), gainToY(table[0]));
            float norm { 0.f };
            for (int i { 0 }; i <= scopeSize; ++i, norm += 1.f/float(scopeSize)) wavePath.lineTo(indexToX(i), gainToY(table[norm]));
            g.setColour(Colours::white.darker());
            g.strokePath(wavePath, PathStrokeType(5.f));
        }
        TableType table;
        Path wavePath;
    };
}
