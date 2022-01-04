#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace sadistic;

sadistic::DeviantEditor::DeviantEditor (Deviant& p) : AudioProcessorEditor (&p), deviant (p) {
    EmpiricalLAF::setDefaultLookAndFeel(&llaf);
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(false, true);
    addAndMakeVisible(renderer);
    renderer.addAndMakeVisible(theDials);
    
    //    if(!deviant.marketplaceStatus.isUnlocked()) {
    //        authorizer = std::make_unique<SadisticUnlockForm>(deviant.marketplaceStatus);
    //        renderer.addAndMakeVisible(*authorizer);
    //        startTimer(1000);
    //    }
    scope.prepare({ deviant.getSampleRate(), (uint32) 2, (uint32) 1 });
    setResizeLimits(150, 75, 1980, 1080);
    setResizable(true, false);
    setSize (600, 300);
}

void sadistic::DeviantEditor::timerCallback() {
    //        if(deviant.marketplaceStatus.isUnlocked()) {
    //            stopTimer();
    //            if(authorizer) authorizer.reset();
    //        }
}

void sadistic::DeviantEditor::resized() {
    auto bounds { getBounds() };
    renderer.setBounds(bounds);
    theDials.setBounds(getBounds());
    //    if(authorizer) authorizer->setBounds(bounds);
}
