#pragma once
#include "deviant.h"
namespace sadistic {
    
    using namespace std::chrono;
    using hi_res = high_resolution_clock;
    
    enum { gain, phase, numModes };
    
    class WavePad : public Component {
    public:
        static constexpr int waveLength { WAVELENGTH };
        WavePad(DeviantScreen& s) : screen(s) {
            setInterceptsMouseClicks(true, true);
        }
        void mouseDown(const MouseEvent&) override { t1 = hi_res::now(); drawWaveFreeHand(); }
        void mouseDrag(const MouseEvent&) override {
            duration<double, std::milli> ms = hi_res::now() - t1;
            if (ms.count() > 100.0) {
                t1 = hi_res::now();
                drawWaveFreeHand();
            }
        }
        float yToGain(float y) { return -(y/(getLocalBounds().getHeight()/2.f) - 1.f);  }
        float gainToY(float gain) { return getLocalBounds().toFloat().getCentreY() - gain * (getLocalBounds().toFloat().getHeight()/2.3f); }
        int xToIndex(float x) { return jlimit(0, waveLength, roundToInt(((x) / float(getWidth())) * float(waveLength))); }
        float indexToX(int i) { return 15.f + (float(i)/float(waveLength)) * (getLocalBounds().toFloat().getWidth() - 25.f); }
        void drawWaveFreeHand() {
            auto point { getMouseXYRelative().toFloat() };
            float g { yToGain(point.y) };
            const int clickSpread { 4 }, index { jlimit(1, waveLength - 2, xToIndex(point.x)) }, leftIndex { jlimit(1, index, index - clickSpread) }, rightIndex { jlimit(index, waveLength - 2, index + clickSpread) };
            screen.wave[index] = g;
            for (int j = index - leftIndex + 1, i = leftIndex; i < index && i < waveLength - 1 && i > 0; ++i, --j) screen.wave[i] = screen.wave[i] + (g - screen.wave[i])/(float(j*4));
            for (int j = rightIndex - index + 1, i = rightIndex; i > index && i < waveLength - 1 && i > 0; --i, --j) screen.wave[i] = screen.wave[i] + (g - screen.wave[i])/(float(j*4));
            screen.updateWaveTable();
            repaint();
        }
        DeviantScreen& screen;
        Path wavePath;
        hi_res::time_point t1 = {};
    };
    
    struct GainPadPad : WavePad {
        GainPadPad(DeviantScreen& s) : WavePad(s) {}
        void paint (Graphics& g) override {
            if (screen.newDataHere) { screen.init(); screen.newDataHere = false; }
            wavePath.clear();
            wavePath.startNewSubPath(indexToX(0), gainToY(screen.wave[0]));
            for (int i { 1 }; i <= waveLength; ++i) wavePath.lineTo(indexToX(i), gainToY(screen.wave[i]));
            g.setColour(Colours::white.darker());
            g.strokePath(wavePath, PathStrokeType(5.f));
        }
    };
    
    struct PhasePadPad : WavePad {
        PhasePadPad(DeviantScreen& s) : WavePad(s) {}
        void paint (Graphics& g) override {
            if (screen.newDataHere) { screen.init(); screen.newDataHere = false; }
            wavePath.clear();
            wavePath.startNewSubPath(indexToX(0), gainToY(screen.gainTable[screen.phaseTable[0]]));
            for (int i { 1 }; i <= waveLength; ++i) wavePath.lineTo(indexToX(i), gainToY(screen.gainTable[screen.phaseTable[i]]));
            g.setColour(Colours::white.darker());
            g.strokePath(wavePath, PathStrokeType(5.f));
        }
    };
    
    struct GainPad : Component, ValueTree::Listener {
        GainPad(DeviantScreen& s) : screen(s), pad(s) {
            getTheTable();
            addAllAndMakeVisible(*this, pad, xAxisLabel, yAxisLabel, button);
            button.label.set("S T A T I C", Colours::black, Colours::grey.darker());
            xAxisLabel.label.setText("A M P L I T U D E   I N", sendNotification);
            yAxisLabel.label.setText("A M P L I T U D E   O U T", sendNotification);
            screen.apvts.state.addListener(this);
        }
        ~GainPad() override { screen.apvts.state.removeListener(this); }
        void valueTreePropertyChanged(ValueTree&, const Identifier& id) override {
            if (id.toString().contains("waveTable")) {
                screen.newDataHere = true;
                if (isVisible()) pad.repaint(); } }
        void resized() override {
            button.setBounds(getWidth() - 105, 5, 95, 15);
            xAxisLabel.setBounds(0, 9 * getHeight()/10, getWidth(), getHeight()/10);
            yAxisLabel.setBounds(0, 0, getHeight()/10, getHeight());
            pad.setBounds(getLocalBounds().reduced(20));
        }
        void setTheTable() { DeviantTree::setWaveTable(screen.apvts, screen.undoManager, screen.wave); }
        void getTheTable() { DeviantTree::getWaveTable(screen.apvts, screen.wave); }
        String wavePadType { "static" };
        SadLabel xAxisLabel { "A M P L I T U D E   I N", false, false, 0.f };
        SadLabel yAxisLabel { "A M P L I T U D E   O U T", true, false, -0.5f };
        DeviantScreen& screen;
        GainPadPad pad;
        SadTextButton button { "S T A T I C" };
    };
    
    struct PhasePad : Component, ValueTree::Listener {
        PhasePad(DeviantScreen& s) : screen(s), pad(s) {
            getTheTable();
            phaseBox.setText("PHASE TABLES");
            for (int i { 1 }; i < Wave<float>::numWaves; ++i) phaseBox.addItem(Wave<float>::getWaveID(i), i);
            phaseBox.onChange = [&,this] {
                if (static_cast<Wave<float>::Type>(phaseBox.getSelectedId()) > 0) {
                Wave<float>::fillTable(s.pWave, s.waveLength, static_cast<Wave<float>::Type>(phaseBox.getSelectedId()), true, true);
                    DeviantTree::setPhaseTable(screen.apvts, screen.undoManager, screen.pWave); } };
            addAllAndMakeVisible(*this, pad, xAxisLabel, yAxisLabel, phaseBox, button);
            button.label.set("D Y N A M I C", Colours::white, Colours::blue.darker());
            xAxisLabel.label.setText("P   H   A   S   E", sendNotification);
            yAxisLabel.label.setText("A M P L I T U D E", sendNotification);
            screen.apvts.state.addListener(this);
        }
        ~PhasePad() override { screen.apvts.state.removeListener(this); }
        void valueTreePropertyChanged(ValueTree&, const Identifier& id) override {
            if (id.toString().contains("phaseTable") || id.toString().contains("gainTable")) {
            screen.newDataHere = true;
            if (isVisible())
                repaint(); } }
        void resized() override {
            button.setBounds(getWidth() - 105, 5, 95, 15);
            xAxisLabel.setBounds(0, 9 * getHeight()/10, getWidth(), getHeight()/10);
            yAxisLabel.setBounds(0, 0, getHeight()/10, getHeight());
            pad.setBounds(getLocalBounds().reduced(20));
            phaseBox.setBounds(5,5,150,20);
        }
        
        void setTheTable() { DeviantTree::setPhaseTable(screen.apvts, screen.undoManager, screen.pWave); DeviantTree::setGainTable(screen.apvts, screen.undoManager, screen.wave); }
        void getTheTable() { DeviantTree::getPhaseTable(screen.apvts, screen.pWave); DeviantTree::getGainTable(screen.apvts, screen.wave); }
        String wavePadType { "dynamic" };
        SadBox phaseBox { "Phase Tables" };
        SadLabel xAxisLabel { "P   H   A   S   E", false, false, 0.f };
        SadLabel yAxisLabel { "A M P L I T U D E", true, false, -0.5f };
        SadTextButton button { "D Y N A M I C" };
        DeviantScreen& screen;
        PhasePadPad pad;
    };
    
//    template<typename WavePadType> struct DynamicPad : Component {
//        static constexpr int waveLength { WAVELENGTH };
//        DynamicPad(DeviantScreen& s, int& pM) : screen(s), pad(screen), currentMode(pM) {
//
//            button.onClick = [&,this]{ initMode(currentMode = (currentMode + 1) % numModes); };
//            addAllAndMakeVisible(*this, pad, xAxisLabel, yAxisLabel, phaseBox, button);
//            initMode(currentMode = pM);
//        }
//        void initMode(int mode) {
//            phaseBox.setVisible(mode == phase ? true : false);
//            getTheTable(screen.apvts, screen.wave);
//            button.label.set(buttonNames[mode], textColors[mode], bgButtonColors[mode]);
//            xAxisLabel.label.setText(xNames[mode], sendNotification);
//            yAxisLabel.label.setText(yNames[mode], sendNotification);
////            gainPad.setVisible(mode == gain ? true : false);
////            phasePad.setVisible(mode == gain ? false : true);
//        }
//
//        void resized() override {
//            button.setBounds(getWidth() - 105, 5, 95, 15);
//            xAxisLabel.setBounds(0, 9 * getHeight()/10, getWidth(), getHeight()/10);
//            yAxisLabel.setBounds(0, 0, getHeight()/10, getHeight());
//            pad.setBounds(getLocalBounds().reduced(20));
////            phasePad.setBounds(getLocalBounds().reduced(20));
//            phaseBox.setBounds(0,0,150,20);
//        }
//        void setTheTable(APVTS& apvts, UndoManager* uM, float* wave) { if (currentMode == phase) { DeviantTree::setPhaseTable(apvts, uM, screen.pWave); DeviantTree::setGainTable(apvts, uM, wave); } else DeviantTree::setWaveTable(apvts, uM, wave); }
//        void getTheTable(APVTS& apvts, float* wave) { if (currentMode == phase) { DeviantTree::getPhaseTable(apvts, screen.pWave); DeviantTree::getGainTable(apvts, wave); } else DeviantTree::getWaveTable(apvts, wave); }
//
//        DeviantScreen& screen;
//        WavePadType pad { screen };
//        String buttonNames[numModes] { "S T A T I C", "D Y N A M I C" };
//        Colour bgButtonColors[numModes] { Colours::grey.darker(), Colours::blue.darker() }, textColors[numModes] { Colours::black, Colours::white };
//        String xNames[numModes] { "A M P L I T U D E   I N", "P   H   A   S   E" };
//        String yNames[numModes] { "A M P L I T U D E   O U T", "A M P L I T U D E" };
//        SadLabel xAxisLabel { "A M P L I T U D E   I N", false, false, 0.f };
//        SadLabel yAxisLabel { "A M P L I T U D E   O U T", true, false, -0.5f };
//
////        GainPad gainPad { screen };
////        PhasePad phasePad { screen };
//        int& currentMode;
//        SadTextButton button { "S T A T I C" };
//    };
}
