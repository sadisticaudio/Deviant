#pragma once
#include "Analyzer.h"
#include "Dials.h"
#include "RoutingMatrix.h"
namespace sadistic {
    
    class DeviantEditor : public AudioProcessorEditor, private Timer {
        static constexpr int scopeSize { SCOPESIZE };
    public:
        DeviantEditor(Deviant&);
        ~DeviantEditor() override;
        void resized() override;
        void activateScreen(int);
        void switchScreen(int);
        void timerCallback() override;
        
    private:
        float wave[scopeSize + 4]{}, pWave[scopeSize + 4]{}, gWave[scopeSize + 4]{};
        Deviant& deviant;
        EmpiricalLAF llaf;
        int guiIdx { dials }, padMode { gain }, dialMode { 1 };
        ScopeBuffer scopeBuffer[numSignals];
        ScopeRenderer renderer { deviant.getAPVTS(), scopeBuffer };
        DualScope scope { deviant.getOscilloscopeFifo(), scopeBuffer };
        Dials theDials { deviant.getTableManager(), dialMode };
        MoreDials<GUITable> theMoreDials { deviant.getTableManager(), deviant.getTableManager().waveTable, nullptr };
        MoreDials<DualGUITable> phaseDials { deviant.getTableManager(), deviant.getTableManager().gainTable, deviant.getTableManager().ptFloat };
        RoutingMatrix routingMatrix { deviant.getTableManager() };
        DeviantScreen* screen[numDisplays] { &routingMatrix, &theDials, &theMoreDials, &phaseDials };
        SadButton toggleButtonLeft { true }, toggleButtonRight { false };
        std::unique_ptr<SadisticUnlockForm> authorizer;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviantEditor)
    };
}
