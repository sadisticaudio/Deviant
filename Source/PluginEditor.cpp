#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace sadistic;

void sadistic::DeviantEditor::activateScreen(int idx){
    renderer.removeChildComponent(screen.get());
    switch (idx) {
        case dials: { screen.release(); screen = std::make_unique<Dials>(deviant.getAPVTS(), deviant.getUndoManager()); } break;
        case staticPad: {
            screen.release(); screen = std::make_unique<MoreDials<GainPad>>(deviant.getAPVTS(), deviant.getUndoManager());
            static_cast<MoreDials<GainPad>*>(screen.get())->pad.button.addListener(this);
            padMode = gain;
        } break;
        case dynamicPad: {
            screen.release();
            screen = std::make_unique<MoreDials<PhasePad>>(deviant.getAPVTS(), deviant.getUndoManager());
            static_cast<MoreDials<PhasePad>*>(screen.get())->pad.button.addListener(this);
            padMode = phase;
        } break;
        case matrix: { screen.release(); screen = std::make_unique<RoutingMatrix>(deviant.getAPVTS(), deviant.getUndoManager()); } break;
        case numDisplays: {} break;
    }
//    screen->init();
    screen->setBounds(getBounds());
    renderer.addAndMakeVisible(*screen);
    toggleButtonLeft.toFront(false);
    toggleButtonRight.toFront(false);
}

void sadistic::DeviantEditor::buttonClicked(Button*) { activateScreen(currentDisplay = (currentDisplay == staticPad ? dynamicPad : staticPad)); }

void sadistic::DeviantEditor::timerCallback() {
    //        if(deviant.marketplaceStatus.isUnlocked()) {
    //            stopTimer();
    //            if(authorizer) authorizer.reset();
    //        }
}

sadistic::DeviantEditor::DeviantEditor (Deviant& p) : AudioProcessorEditor (&p), deviant (p) {
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(false, true);
    addAndMakeVisible(renderer);
    activateScreen(currentDisplay);
    renderer.addAndMakeVisible(toggleButtonLeft);
    renderer.addAndMakeVisible(toggleButtonRight);
    toggleButtonLeft.onClick = [&] {
        if (currentDisplay == dials) currentDisplay = matrix;
        else if (currentDisplay == staticPad) currentDisplay = dials;
        else if (currentDisplay == dynamicPad) currentDisplay = dials;
        else if (currentDisplay == matrix) currentDisplay = padMode == gain ? staticPad : dynamicPad;
        activateScreen(currentDisplay); };
    toggleButtonRight.onClick = [&] {
        if (currentDisplay == dials) currentDisplay = padMode == gain ? staticPad : dynamicPad;
        else if (currentDisplay == staticPad) currentDisplay = matrix;
        else if (currentDisplay == dynamicPad) currentDisplay = matrix;
        else if (currentDisplay == matrix) currentDisplay = dials;
        activateScreen(currentDisplay); };
//    savePresetButton.onClick = [&] {
//        juce::File outputFile(getWaveTableFile("defaultWaveTable"));
//        outputFile.deleteFile();
//        MemoryBlock block { deviant.waveTableBuffer.getReadableBuffer(), BUFFERLENGTH * sizeof(float) };
//        outputFile.replaceWithData(deviant.waveTableBuffer.getReadableBuffer(), BUFFERLENGTH * sizeof(float));
//        deviant.saveWaveTable();
//    };
//    if(!deviant.marketplaceStatus.isUnlocked()) {
//        authorizer = std::make_unique<SadisticUnlockForm>(deviant.marketplaceStatus);
//        renderer.addAndMakeVisible(*authorizer);
//        startTimer(1000);
//    }
    
    scope.prepare({deviant.getSampleRate(), (uint32) 2, (uint32) 1});
    renderer.start();
    setResizeLimits(150, 75, 1980, 1080);
    setResizable(true, false);
    setSize (600, 300);
}

sadistic::DeviantEditor::~DeviantEditor () { renderer.stop(); }

void sadistic::DeviantEditor::resized() {
    auto bounds { getBounds() };
    renderer.setBounds(bounds);
    auto toggleWidth { jmax(10,getWidth()/20) }, toggleHeight { jmax(20,bounds.getHeight()/20) };
    toggleButtonRight.setBounds(bounds.getCentreX() + 10, bounds.getY() + 4, toggleWidth, toggleHeight);
    toggleButtonLeft.setBounds(bounds.getCentreX() - (10 + toggleWidth), bounds.getY() + 4, toggleWidth, toggleHeight);
    screen->setBounds(bounds);
//    if(authorizer) authorizer->setBounds(bounds);
}
