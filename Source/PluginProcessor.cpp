#include "PluginProcessor.h"
#include "PluginEditor.h"
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new sadistic::Deviant({}); }
sadistic::Deviant::Deviant(AudioProcessorValueTreeState::ParameterLayout layout) : AudioProcessor (getDefaultBusesProperties()), mgmt(apvts, &undoManager), membersD(layout, apvts), membersF(membersD), apvts(*this, &undoManager, "PARAMETERS", std::move(layout)) {
    marketplaceStatus.load();
    
    setGainTable(); setWaveTable();
    double def[WAVELENGTH + 1];
    Wave<double>::fillTable(def, WAVELENGTH, Wave<double>::sine, true, true);
    DeviantTree::setPhaseTable(apvts, &undoManager, def);

    //this is where all listeners should be added, if possible.  doing so prior means the state might not be ready
    apvts.state.addListener(&membersD.staticWaveShaper);
    apvts.state.addListener(&membersD.dynamicWaveShaper);
    apvts.state.addListener(&membersF.staticWaveShaper);
    apvts.state.addListener(&membersF.dynamicWaveShaper);
    
    listenToTheseParameters(apvts, this,
                            "staticAtanDrive", "staticAtanGain",
                            "staticBitCrusherDrive", "staticBitCrusherFloor",
                            "staticDeviationDrive", "staticDeviationSaturation", "staticDeviationGate",
                            
                            "staticAtanBlend", "staticBitCrusherBlend", "staticDeviationBlend",
                            
                            "dynamicAtanDrive", "dynamicAtanGain",
                            "dynamicBitCrusherDrive", "dynamicBitCrusherFloor",
                            "dynamicDeviationDrive", "dynamicDeviationSaturation", "dynamicDeviationGate",
                            
                            "dynamicAtanBlend", "dynamicBitCrusherBlend", "dynamicDeviationBlend",
                            
                            "dynamicAtanRoute", "dynamicBitCrusherRoute", "dynamicDeviationRoute", "dynamicWaveShaperRoute", "filterARoute", "filterBRoute", "staticAtanRoute", "staticBitCrusherRoute", "staticDeviationRoute", "staticWaveShaperRoute",
                            
                            "dynamicAtanIndex", "dynamicBitCrusherIndex", "dynamicDeviationIndex", "dynamicWaveShaperIndex", "filterAIndex", "filterBIndex", "staticAtanIndex", "staticBitCrusherIndex", "staticDeviationIndex", "staticWaveShaperIndex");
    setGainTable(); setWaveTable();
    
//    struct Printer : Timer {
//        void timerCallback() override {
//            print(membersF)
//        }
//    };
}
void sadistic::Deviant::prepareToPlay (double sR, int sPB) { if (getProcessingPrecision() == doublePrecision) prepare(sR, sPB, membersD); else prepare(sR, sPB, membersF); };
//void sadistic::Deviant::processBlock (AudioBuffer<double>& buffer, MidiBuffer&) { membersD.process(buffer, [&, this](AudioBuffer<double>& buf) { processTheDamnBlock(buf, membersD); }); }
//void sadistic::Deviant::processBlock (AudioBuffer<float>& buffer, MidiBuffer&) { membersF.process(buffer, [&, this](AudioBuffer<float>& buf) { processTheDamnBlock(buf, membersF); }); }
void sadistic::Deviant::processBlock (AudioBuffer<float>& buffer, MidiBuffer&) { processTheDamnBlock(buffer, membersF); }
void sadistic::Deviant::processBlock (AudioBuffer<double>& buffer, MidiBuffer&) { processTheDamnBlock(buffer, membersD); }
AudioProcessorEditor* sadistic::Deviant::createEditor() { return new sadistic::DeviantEditor (*this); }
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
