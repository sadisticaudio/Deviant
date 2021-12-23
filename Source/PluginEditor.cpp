#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace sadistic;

sadistic::DeviantEditor::DeviantEditor (Deviant& p) : AudioProcessorEditor (&p), deviant (p) {
    EmpiricalLAF::setDefaultLookAndFeel(&llaf);
    guiIdx = deviant.getCurrentScreen();
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(false, true);
    addAndMakeVisible(renderer);
    activateScreen(guiIdx);
    renderer.addAndMakeVisible(toggleButtonLeft);
    renderer.addAndMakeVisible(toggleButtonRight);
    toggleButtonLeft.onClick = [&] { switchScreen(guiIdx == dials ? matrix : (guiIdx == staticPad || guiIdx == dynamicPad) ? dials : padMode == gain ? staticPad : dynamicPad); };
    toggleButtonRight.onClick = [&] { switchScreen(guiIdx == dials ? padMode == gain ? staticPad : dynamicPad : guiIdx == matrix ? dials : matrix); };
    theMoreDials.button.onClick = [&] { padMode = phase; switchScreen(dynamicPad); };
    phaseDials.button.onClick = [&] { padMode = gain; switchScreen(staticPad); };
    //    if(!deviant.marketplaceStatus.isUnlocked()) {
    //        authorizer = std::make_unique<SadisticUnlockForm>(deviant.marketplaceStatus);
    //        renderer.addAndMakeVisible(*authorizer);
    //        startTimer(1000);
    //    }
    startTimerHz(40);
    scope.prepare({ deviant.getSampleRate(), (uint32) 2, (uint32) 1 });
    setResizeLimits(150, 75, 1980, 1080);
    setResizable(true, false);
    setSize (600, 300);
}

sadistic::DeviantEditor::~DeviantEditor(){
    EmpiricalLAF::setDefaultLookAndFeel(nullptr);
    stopTimer();
}

void sadistic::DeviantEditor::switchScreen(int idx){
    static_cast<Component*>(screen[guiIdx])->setVisible(false);
    renderer.removeChildComponent(static_cast<Component*>(screen[guiIdx]));
    activateScreen(guiIdx = idx);
}

void sadistic::DeviantEditor::activateScreen(int idx){
    deviant.setCurrentScreen(idx);
    renderer.addAndMakeVisible(screen[guiIdx]);
    
    // these need to always be in front to be clickable after the display changes
    toggleButtonLeft.toFront(false);
    toggleButtonRight.toFront(false);
    renderer.repaint();
}

void sadistic::DeviantEditor::timerCallback() {
    //        if(deviant.marketplaceStatus.isUnlocked()) {
    //            stopTimer();
    //            if(authorizer) authorizer.reset();
    //        }
    if (deviant.getTableManager().newGUIDataHere) renderer.repaint();
}

void sadistic::DeviantEditor::resized() {
    auto bounds { getBounds() };
    renderer.setBounds(bounds);
    auto toggleWidth { jmax(10,getWidth()/20) }, toggleHeight { jmax(20,bounds.getHeight()/20) };
    toggleButtonRight.setBounds(bounds.getCentreX() + 10, bounds.getY() + toggleHeight, toggleWidth, toggleHeight);
    toggleButtonLeft.setBounds(bounds.getCentreX() - (10 + toggleWidth), bounds.getY() + toggleHeight, toggleWidth, toggleHeight);
    for (auto* s : screen) s->setBounds(getBounds());
    //    if(authorizer) authorizer->setBounds(bounds);
}
