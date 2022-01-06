#include "PluginProcessor.h"
#include "PluginEditor.h"
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new sadistic::Deviant({}); }

sadistic::Deviant::Deviant(AudioProcessorValueTreeState::ParameterLayout layout) : AudioProcessor (getDefaultBusesProperties()), membersD(layout, cIdx, coefficients), membersF(membersD), apvts(*this, &undoManager, "PARAMETERS", std::move(layout)) {
    apvts.state.setProperty (IDs::currentScreen, var(int(0)), nullptr);
    marketplaceStatus.load();
    membersF.init();
    membersD.init();
}
void sadistic::Deviant::prepareToPlay (double sR, int sPB) { if (getProcessingPrecision() == doublePrecision) prepare(sR, sPB, membersD); else prepare(sR, sPB, membersF); };
void sadistic::Deviant::processBlock (AudioBuffer<float>& buffer, MidiBuffer&) { processTheDamnBlock(buffer, membersF); }
void sadistic::Deviant::processBlock (AudioBuffer<double>& buffer, MidiBuffer&) { processTheDamnBlock(buffer, membersD); }
void sadistic::Deviant::processBlockBypassed (AudioBuffer<float>& b, MidiBuffer&) { processTheDamnBlock(b, membersF, true); }
void sadistic::Deviant::processBlockBypassed (AudioBuffer<double>& b, MidiBuffer&) { processTheDamnBlock(b, membersD, true); }

bool sadistic::Deviant::canApplyBusCountChange (bool, bool, BusProperties&){ return true; }

bool sadistic::Deviant::canAddBus (bool isInput) const {
    if (getBusCount(true) < 99 && isInput == true) return true;
    if (getBusCount(false) == 0 && isInput == false) return true;
    return false;
}

bool sadistic::Deviant::canRemoveBus (bool isInput) const {
    if (getBusCount(isInput) > 1) return true;
    else return false;
}

bool sadistic::Deviant::isBusesLayoutSupported (const BusesLayout& layout) const {
    //If main buses are both either one or two channels, we are good
    if (layout.getMainInputChannelSet() == layout.getMainOutputChannelSet() &&
        layout.inputBuses.size() == 1 &&
        layout.outputBuses.size() == 1)
        return true;
    return false;
}

AudioProcessorEditor* sadistic::Deviant::createEditor() { return createDeviantEditor(*this); }
