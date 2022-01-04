#pragma once
#include "Analyzer.h"
#include "Renderer.h"
#include "Dials.h"

namespace sadistic {
    
    class DeviantEditor : public AudioProcessorEditor, private Timer {
    public:
        DeviantEditor(Deviant&);
        ~DeviantEditor() override{}
        void resized() override;
        void timerCallback() override;
        
    private:
        Deviant& deviant;
        EmpiricalLAF llaf;
        ScopeBuffer scopeBuffer[numSignals];
        ScopeRenderer renderer { deviant.getAPVTS(), scopeBuffer };
        DualScope scope { deviant.getOscilloscopeFifo(), scopeBuffer };
        Dials theDials { deviant.getAPVTS(), deviant.getAPVTS().state.getProperty(IDs::currentScreen) };
        SadButton toggleButtonLeft { true }, toggleButtonRight { false };
        std::unique_ptr<SadisticUnlockForm> authorizer;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviantEditor)
    };
}
