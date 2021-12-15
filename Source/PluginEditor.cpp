#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace sadistic;

void sadistic::DeviantEditor::switchScreen(int idx){
//    deviant.getAPVTS().state.removeListener(screen.get());
//    deviant.getAPVTS().removeParameterListener("waveTableID", screen.get());
    renderer.removeChildComponent(screen.get());
    activateScreen(idx);
}

void sadistic::DeviantEditor::activateScreen(int idx){
    
    switch (idx) {
        case dials:         { screen.reset(new Dials(hub, dialMode));
            static_cast<Dials*>(screen.get())->button.onClick = [this]{ ++dialMode%=numFX;  switchScreen(dials); };
        } break;
        case staticPad:     { screen.reset(new MoreDials<GainPad>(hub));
            static_cast<MoreDials<GainPad>*>(screen.get())->button.onClick = [this]{ switchScreen(dynamicPad); };
            padMode = gain;
        } break;
        case dynamicPad:    { screen.reset(new MoreDials<PhasePad>(hub));
            static_cast<MoreDials<PhasePad>*>(screen.get())->button.onClick = [this]{  switchScreen(staticPad); };
            padMode = phase;
        } break;
        case matrix:        { screen.reset(new RoutingMatrix(hub)); } break;
        case numDisplays: {} break;
    }
    
//    deviant.getAPVTS().state.addListener(screen.get());
//    deviant.getAPVTS().addParameterListener("waveTableID", screen.get());
    deviant.setCurrentScreen(idx);
    screen->setBounds(getBounds());
    screen->setLookAndFeel(&llaf);
    renderer.addAndMakeVisible(*screen);
    
    // these need to always be in front to be clickable after the display changes
    toggleButtonLeft.toFront(false);
    toggleButtonRight.toFront(false);
}

void sadistic::DeviantEditor::timerCallback() {
    //        if(deviant.marketplaceStatus.isUnlocked()) {
    //            stopTimer();
    //            if(authorizer) authorizer.reset();
    //        }
}

sadistic::DeviantEditor::DeviantEditor (Deviant& p) : AudioProcessorEditor (&p), deviant (p) {
    currentDisplay = deviant.getCurrentScreen();
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(false, true);
    addAndMakeVisible(renderer);
    activateScreen(currentDisplay);
    renderer.addAndMakeVisible(toggleButtonLeft);
    renderer.addAndMakeVisible(toggleButtonRight);
//    toggleButtonLeft.onClick = [&] { switchScreen((padMode + numDisplays - (currentDisplay == dynamicPad ? 2 : 1)) % numDisplays); };
//    toggleButtonRight.onClick = [&] { switchScreen((padMode + (currentDisplay == staticPad ? 2 : 1)) % numDisplays); };
    toggleButtonLeft.onClick = [&] {
        if (currentDisplay == dials) currentDisplay = matrix;
        else if (currentDisplay == staticPad) currentDisplay = dials;
        else if (currentDisplay == dynamicPad) currentDisplay = dials;
        else if (currentDisplay == matrix) currentDisplay = padMode == gain ? staticPad : dynamicPad;
        switchScreen(currentDisplay); };
    toggleButtonRight.onClick = [&] {
        if (currentDisplay == dials) currentDisplay = padMode == gain ? staticPad : dynamicPad;
        else if (currentDisplay == staticPad) currentDisplay = matrix;
        else if (currentDisplay == dynamicPad) currentDisplay = matrix;
        else if (currentDisplay == matrix) currentDisplay = dials;
        switchScreen(currentDisplay); };
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

sadistic::DeviantEditor::~DeviantEditor(){
//    deviant.getAPVTS().state.removeListener(screen.get());
    
}

void sadistic::DeviantEditor::resized() {
    auto bounds { getBounds() };
    renderer.setBounds(bounds);
    auto toggleWidth { jmax(10,getWidth()/20) }, toggleHeight { jmax(20,bounds.getHeight()/20) };
    toggleButtonRight.setBounds(bounds.getCentreX() + 10, bounds.getY() + toggleHeight, toggleWidth, toggleHeight);
    toggleButtonLeft.setBounds(bounds.getCentreX() - (10 + toggleWidth), bounds.getY() + toggleHeight, toggleWidth, toggleHeight);
    screen->setBounds(bounds);
    //    if(authorizer) authorizer->setBounds(bounds);
}

//void sadistic::DeviantEditor::activateScreen(int idx){
//    renderer.removeChildComponent(screen.get());
//    switch (idx) {
//        case dials: { screen.release(); screen = std::make_unique<Dials>(deviant.getAPVTS(), deviant.getUndoManager()); } break;
//        case staticPad: {
//            screen.release(); screen = std::make_unique<MoreDials<GainPad>>(deviant.getAPVTS(), deviant.getUndoManager());
//            static_cast<MoreDials<GainPad>*>(screen.get())->pad.button.addListener(this);
//            padMode = gain;
//        } break;
//        case dynamicPad: {
//            screen.release();
//            screen = std::make_unique<MoreDials<PhasePad>>(deviant.getAPVTS(), deviant.getUndoManager());
//            static_cast<MoreDials<PhasePad>*>(screen.get())->pad.button.addListener(this);
//            padMode = phase;
//        } break;
//        case matrix: { screen.release(); screen = std::make_unique<RoutingMatrix>(deviant.getAPVTS(), deviant.getUndoManager()); } break;
//        case numDisplays: {} break;
//    }
////    screen->init();
//    screen->setBounds(getBounds());
//    renderer.addAndMakeVisible(*screen);
//    toggleButtonLeft.toFront(false);
//    toggleButtonRight.toFront(false);
//}
//
//void sadistic::DeviantEditor::buttonClicked(Button*) { activateScreen(currentDisplay = (currentDisplay == staticPad ? dynamicPad : staticPad)); }
//
//void sadistic::DeviantEditor::timerCallback() {
//    //        if(deviant.marketplaceStatus.isUnlocked()) {
//    //            stopTimer();
//    //            if(authorizer) authorizer.reset();
//    //        }
//}
//
//sadistic::DeviantEditor::DeviantEditor (Deviant& p) : AudioProcessorEditor (&p), deviant (p) {
//    setWantsKeyboardFocus(true);
//    setInterceptsMouseClicks(false, true);
//    addAndMakeVisible(renderer);
//    activateScreen(currentDisplay);
//    renderer.addAndMakeVisible(toggleButtonLeft);
//    renderer.addAndMakeVisible(toggleButtonRight);
//    toggleButtonLeft.onClick = [&] {
//        if (currentDisplay == dials) currentDisplay = matrix;
//        else if (currentDisplay == staticPad) currentDisplay = dials;
//        else if (currentDisplay == dynamicPad) currentDisplay = dials;
//        else if (currentDisplay == matrix) currentDisplay = padMode == gain ? staticPad : dynamicPad;
//        activateScreen(currentDisplay); };
//    toggleButtonRight.onClick = [&] {
//        if (currentDisplay == dials) currentDisplay = padMode == gain ? staticPad : dynamicPad;
//        else if (currentDisplay == staticPad) currentDisplay = matrix;
//        else if (currentDisplay == dynamicPad) currentDisplay = matrix;
//        else if (currentDisplay == matrix) currentDisplay = dials;
//        activateScreen(currentDisplay); };
////    savePresetButton.onClick = [&] {
////        juce::File outputFile(getWaveTableFile("defaultWaveTable"));
////        outputFile.deleteFile();
////        MemoryBlock block { deviant.waveTableBuffer.getReadableBuffer(), BUFFERLENGTH * sizeof(float) };
////        outputFile.replaceWithData(deviant.waveTableBuffer.getReadableBuffer(), BUFFERLENGTH * sizeof(float));
////        deviant.saveWaveTable();
////    };
////    if(!deviant.marketplaceStatus.isUnlocked()) {
////        authorizer = std::make_unique<SadisticUnlockForm>(deviant.marketplaceStatus);
////        renderer.addAndMakeVisible(*authorizer);
////        startTimer(1000);
////    }
//
//    scope.prepare({deviant.getSampleRate(), (uint32) 2, (uint32) 1});
//    setResizeLimits(150, 75, 1980, 1080);
//    setResizable(true, false);
//    setSize (600, 300);
//}
//
//sadistic::DeviantEditor::~DeviantEditor () { }
//
//void sadistic::DeviantEditor::resized() {
//    auto bounds { getBounds() };
//    renderer.setBounds(bounds);
//    auto toggleWidth { jmax(10,getWidth()/20) }, toggleHeight { jmax(20,bounds.getHeight()/20) };
//    toggleButtonRight.setBounds(bounds.getCentreX() + 10, bounds.getY() + 4, toggleWidth, toggleHeight);
//    toggleButtonLeft.setBounds(bounds.getCentreX() - (10 + toggleWidth), bounds.getY() + 4, toggleWidth, toggleHeight);
//    screen->setBounds(bounds);
////    if(authorizer) authorizer->setBounds(bounds);
//}
