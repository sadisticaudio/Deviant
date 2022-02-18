#include "PluginProcessor.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new sadistic::Deviant({}); }

sadistic::Deviant::Deviant(AudioProcessorValueTreeState::ParameterLayout layout) : AudioProcessor (getDefaultBusesProperties()), membersD(layout), membersF(membersD), apvts(*this, &undoManager, "PARAMETERS", std::move(layout)) {
    apvts.state.setProperty (Identifier("currentScreen"), var(int(0)), nullptr);
    marketplaceStatus.load();
    membersF.init();
    membersD.init();
}

AudioProcessorEditor* sadistic::Deviant::createEditor() { return createDeviantEditor(*this); }
