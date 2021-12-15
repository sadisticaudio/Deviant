#pragma once
#include "Analyzer.h"
#include "Dials.h"
#include "RoutingMatrix.h"
#include "WavePad.h"
namespace sadistic {
    
    class DeviantEditor : public AudioProcessorEditor, private Timer {
    public:
        DeviantEditor(Deviant&);
        ~DeviantEditor() override;
        void resized() override;
        void activateScreen(int);
        void switchScreen(int);
        void timerCallback() override;
        
    private:
        Deviant& deviant;
        DeviantGUIHub hub { deviant.getTableManager() };
        EmpiricalLAF llaf;
        int currentDisplay { dials }, padMode { gain }, dialMode { 1 };
        ScopeBuffer scopeBuffer[numSignals];
        ScopeRenderer renderer { deviant.getAPVTS(), scopeBuffer };
        DualScope scope { deviant.getOscilloscopeFifo(), scopeBuffer };
        std::unique_ptr<DeviantScreen> screen { nullptr };
        SadButton toggleButtonLeft { true }, toggleButtonRight { false };
        std::unique_ptr<SadisticUnlockForm> authorizer;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviantEditor)
    };
    
//    class DeviantEditor : public AudioProcessorEditor, public Button::Listener, private Timer {
//    public:
//        DeviantEditor (Deviant&);
//        ~DeviantEditor () override;
//        void resized() override;
//        void activateScreen(int);
//        void timerCallback() override;
//        void buttonClicked(Button*) override;
//
//    private:
//        Deviant& deviant;
//        DeviantGUIHub hub { deviant.getTableManager() };
//        EmpiricalLAF llaf;
//        enum { matrix, dials, staticPad, dynamicPad, numDisplays };
//        int currentDisplay { dials };
//        int padMode { gain };
//        ScopeBuffer scopeBuffer[numSignals];
//        ScopeRenderer renderer { deviant.getAPVTS(), scopeBuffer };
//        DualScope scope { deviant.getOscilloscopeFifo(), scopeBuffer };
//        std::unique_ptr<DeviantScreen> screen { nullptr };
//        SadButton toggleButtonLeft { true }, toggleButtonRight { false };
//        std::unique_ptr<SadisticUnlockForm> authorizer;
//        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviantEditor)
//    };
}
