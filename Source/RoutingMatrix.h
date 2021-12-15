#pragma once
#include "deviant.h"
namespace sadistic {
    
    using namespace std::chrono;
    using hi_res = high_resolution_clock;
    
    struct RoutingMatrix : DragAndDropContainer, DeviantScreen {
        
        struct Icon : public Component {
            static constexpr int iconResolution { 50 };
            Icon(const String eID) : effectID(eID) {
                //                setInterceptsMouseClicks(false, false);
                if(effectID.contains("static")) {
                    Wave<float>::fillTable(wave, iconResolution, Wave<float>::sine);
                    for (auto& i : wave) { i *= -0.3f; i += 0.5f; }
                }
                else if (effectID.contains("dynamic")) {
                    Wave<float>::fillTable(wave, iconResolution, Wave<float>::sine);
                    float offset { 0.7f }, step { -1.4f / float(iconResolution) };
                    for (int i { 0 }; i < iconResolution; ++i, offset += step) wave[i] += offset;
                    for (auto& i : wave) { i *= -0.3f; i += 0.5f; }
                }
                path.preallocateSpace(iconResolution * 3 + 100);
                rectangle.preallocateSpace(40);
            }
            void paint(Graphics& g) override {
                auto bounds { getLocalBounds().toFloat() };
                float w { bounds.getWidth() }, h { bounds.getHeight() };
                
                path.clear();
                rectangle.clear();
                
                if(effectID.contains("filter")) {
                    float startGap { range.convertTo0to1(low) * 0.6f };
                    float endGap { (1.f - range.convertTo0to1(high)) * 0.6f };
                    path.startNewSubPath(w * (0.1f + startGap), h);
                    path.lineTo(w * (0.1f + startGap), h * 0.4f);
                    path.quadraticTo(w * (0.1f + startGap), h * 0.3f, w * (0.2f + startGap), h * 0.3f);
                    path.lineTo(w * (0.8f - endGap), h * 0.3f);
                    path.quadraticTo(w * (0.8f - endGap), h * 0.3f, w * (0.9f - endGap), h * 0.4f);
                    path.lineTo(w * (0.9f - endGap), h);
                    //                    path.lineTo(w * 0.9f, h);
                    g.setColour(Colours::steelblue.darker().withAlpha(0.4f));
                    g.fillPath(path);
                    g.setColour(Colours::steelblue);
                    g.strokePath(path, PathStrokeType(3.f));
                    g.setColour(colour);
                    rectangle.addRoundedRectangle(bounds, 10);
                    g.strokePath(rectangle, PathStrokeType(3.f));
                }
                else {
                    path.startNewSubPath(0.f, wave[0] * h);
                    for (int i = 1; i < iconResolution; ++i) path.lineTo(float(i)/float(iconResolution) * w, wave[i] * h);
                    g.setColour(Colours::steelblue);
                    g.strokePath(path, PathStrokeType(3.f));
                    g.setColour(colour);
                    rectangle.addRoundedRectangle(bounds, 10);
                    g.strokePath(rectangle, PathStrokeType(3.f));
                }
            }
            Colour colour { Colours::grey };
            String effectID;
            float wave[iconResolution]{}, low { 20.f }, high { 20000.f };
            Path path, rectangle;
            NormalisableRange<float> range { 20.f, 20000.f, 0.f, 0.4f };
        };
        
        struct Effect;
        
        struct InsertionPoint : Component, DragAndDropTarget {
            InsertionPoint(RoutingMatrix& m, int r = 0, Effect* e = nullptr, Colour c = Colours::blue.withAlpha(0.f)) : matrix(m), route(r), effect(e), colour(c) { setInterceptsMouseClicks(true, true); }
            bool isInterestedInDragSource (const SourceDetails& /*dragSourceDetails*/) override { return true; }
            void itemDragEnter (const SourceDetails& /*dragSourceDetails*/) override;
            void itemDragExit (const SourceDetails& /*dragSourceDetails*/) override;
            void itemDropped (const SourceDetails& details) override;
            void paint (Graphics& g) override { g.fillAll(colour); }
            void setTransparency(float alpha) { colour = Colours::blue.withAlpha(alpha); }
            Effect* getEffectUnderMouse(Point<float> pt);
            //            void mouseMove(const MouseEvent& e) override {
            //                auto* newEffect { getEffectUnderMouse(e.position) };
            //                if (newEffect && newEffect != effectUnderMouse) {
            //                    newEffect->lighten();//mouseEnter(e);
            //                    if (effectUnderMouse) effectUnderMouse->darken();//mouseExit(e);
            //                    effectUnderMouse = newEffect;
            //                }
            //            }
            //            void mouseExit(const MouseEvent& e) override {
            //                if (effectUnderMouse != getEffectUnderMouse(e.position)) if(effectUnderMouse) effectUnderMouse->darken();//mouseExit(e);
            //                effectUnderMouse = getEffectUnderMouse(e.position);
            //            }
            RoutingMatrix& matrix;
            int route;
            Effect* effect { nullptr };
            Effect* effectUnderMouse { nullptr };
            Colour colour;
        };
        
        struct Effect : Component {
            Effect(RoutingMatrix& r, const String eID, APVTS& apvts) :
            effectID(eID), matrix(r), insertionPoint(matrix, 0, this), enabledAttachment(apvts, translate(eID + "Enabled"), enabledSlider), routeAttachment(apvts, translate(eID + "Route"), routeSlider), indexAttachment(apvts, translate(eID + "Index"), indexSlider), blendAttachment(apvts, translate(eID + "Blend"), blendSlider), icon(eID) {
                addAndMakeVisible(icon);
                setAlpha((bool)enabledSlider.getValue() ? 1.f : 0.5f);
            }
            void resized() override { icon.setBounds(getLocalBounds().reduced(15)); }
            String getID() { return effectID; }
            void mouseDrag (const MouseEvent& e) override;
            void mouseDown(const MouseEvent& e) override;
            void mouseUp(const MouseEvent& e) override;
            void setSVG    (const char* svg, const int svgSize) { d.loadPathFromData(svg, (size_t) svgSize); }
            void lighten() { icon.colour = Colours::grey.brighter(); repaint(); }
            void darken() { icon.colour = Colours::grey; repaint(); }
            const String effectID;
            RoutingMatrix& matrix;
            InsertionPoint insertionPoint;
            Slider enabledSlider, routeSlider, indexSlider, blendSlider;
            APVTS::SliderAttachment enabledAttachment, routeAttachment, indexAttachment, blendAttachment;
            hi_res::time_point t1 = {}, t2 = {};
            Icon icon;
            Path d;
        };
        
        struct EffectRoutingState {
            bool operator< (const EffectRoutingState&) const;
            explicit operator bool() const { return ePtr != nullptr; }
            Effect* operator->() { return ePtr; }
            Effect* get() { return ePtr; }
            Effect& operator*() { return *ePtr; }
            Effect* ePtr; InsertionPoint* iPtr; bool enabled{}; int route{}; int index{}; double blend{};
        };
        
        enum { preRoute, upperRoute, lowerRoute, postRoute, numRoutes };
        
        RoutingMatrix(DeviantGUIHub& h) : DeviantScreen(h) {
            
            EffectRoutingState tempFX[numFX];
            for (int i { 0 }; i < numFX; ++i) {
                tempFX[i] = { &effects[i], &effects[i].insertionPoint, static_cast<bool>(effects[i].enabledSlider.getValue()), static_cast<int>(effects[i].routeSlider.getValue()), static_cast<int>(effects[i].indexSlider.getValue()), effects[i].blendSlider.getValue() };
            }
            std::sort(tempFX, tempFX + numFX);
            for (int i { 0 }; i < numFX; ++i) fx[tempFX[i].route].add(tempFX[i]);
            for (int j { 0 }; j < numRoutes; ++j) { fx[j].add({ nullptr, &endPoint[j] }); for(auto f : fx[j]) addAndMakeVisible(f.iPtr); }
            
            for (int i { 0 }; i < numFX; ++i) { addAndMakeVisible(effects[i]); effects[i].addMouseListener(this, true); }
            
            knobs[0].onValueChange = [&,this]() { effects[4].icon.low = float(knobs[0].getValue()); repaint(); };
            knobs[1].onValueChange = [&,this]() { effects[4].icon.high = float(knobs[1].getValue()); repaint(); };
            knobs[2].onValueChange = [&,this]() { effects[5].icon.low = float(knobs[2].getValue()); repaint(); };
            knobs[3].onValueChange = [&,this]() { effects[5].icon.high = float(knobs[3].getValue()); repaint(); };
            
            effects[4].icon.low = float(knobs[0].getValue());
            effects[4].icon.high = float(knobs[1].getValue());
            effects[5].icon.low = float(knobs[2].getValue());
            effects[5].icon.high = float(knobs[3].getValue());
        }
        
        int getNumInMiddle() { return jmax(1, fx[1].size()-1, fx[2].size()-1); }
        int getNumColumns() { return getNumInMiddle() + jmax(1, fx[0].size()-1) + jmax(1, fx[3].size()-1); }
        void moveEffect(Effect* effectComponent, int newRow, int newIndex) {
            const int oldRow { static_cast<int>(effectComponent->routeSlider.getValue()) }, oldIndex { static_cast<int>(effectComponent->indexSlider.getValue()) };
            
            fx[newRow].insert(newIndex, fx[oldRow].removeAndReturn(oldIndex));
            
            int w { getWidth()/getNumColumns() }, boxWidth { static_cast<int>(float(w) * 0.75f) };
            
            if(oldRow != newRow) {
                for (auto j : fx) for(auto f : j) f.iPtr->setSize(w,w);
                for (auto& e : effects) e.setSize(boxWidth, boxWidth);
            }
            for (int row = 0; row < numRoutes; ++row) {
                for (int i { 0 }; i < fx[row].size()-1; ++i) {
                    if (fx[row][i].index != i) { String iString { fx[row][i]->effectID + "Index" }; hub.mgmt.apvts.getParameter(iString)->beginChangeGesture(); fx[row][i]->indexSlider.setValue(i); }
                    if (fx[row][i].route != row) { String rString { fx[row][i]->effectID + "Route" }; hub.mgmt.apvts.getParameter(rString)->beginChangeGesture(); fx[row][i]->routeSlider.setValue(row); }
                }
            }
            for (int row = 0; row < numRoutes; ++row) {
                for (int i { 0 }; i < fx[row].size()-1; ++i) {
                    if (fx[row][i].index != i) { String iString { fx[row][i]->effectID + "Index" }; hub.mgmt.apvts.getParameter(iString)->endChangeGesture(); fx[row].getReference(i).index = i;  }
                    if (fx[row][i].route != row) { String rString { fx[row][i]->effectID + "Route" }; hub.mgmt.apvts.getParameter(rString)->endChangeGesture(); fx[row].getReference(i).route = row;  }
                }
            }
            layOutItems();
        }
        
        void layOutItems() {
            auto w { 2 * getHeight()/12 }, h { getHeight()/8 };
            for (int i { 0 }; i < numSliders; ++i)  {
                frames[i].setSize(w,h);
                frames[i].setCentrePosition(i%2 * w * 2 + w/2, i < 2 ? h/2 : getHeight() - h/2);
                knobs[i].setBounds(frames[i].getBounds());
                knobs[i].setTextBoxStyle(knobs[i].isLeft ? Slider::TextBoxLeft : Slider::TextBoxRight, false, h, h);
            }
            frames[numSliders].setBounds(knobs[0].getBounds().getRight(), knobs[0].getY(), w, h);
            frames[numSliders + 1].setBounds(knobs[2].getBounds().getRight(), knobs[2].getY(), w, h);
            labels[0].setBounds(frames[numSliders].getBounds());
            labels[1].setBounds(frames[numSliders + 1].getBounds());
            routeBounds[upperRoute] = getLocalBounds().reduced(margin, 0);
            routeBounds[preRoute]   = routeBounds[upperRoute].removeFromLeft(getWidth()/getNumColumns() * jmax(fx[0].size()-1, 1));
            routeBounds[postRoute]  = routeBounds[upperRoute].removeFromRight(getWidth()/getNumColumns() * jmax(fx[3].size()-1, 1));
            routeBounds[lowerRoute] = routeBounds[upperRoute].removeFromBottom(getHeight()/2);
            
            for (int slotWidth, position, row = 0; row < numRoutes; ++row) {
                if(fx[row].size() > 1) {
                    slotWidth = routeBounds[row].getWidth() / (fx[row].size() - 1), position = routeBounds[row].getX() + slotWidth/2;
                    for (int i { 0 }; i < fx[row].size(); ++i, position += slotWidth) {
                        if(fx[row][i]) fx[row][i]->setCentrePosition(position, routeBounds[row].getCentreY());
                        fx[row][i].iPtr->setCentrePosition(position - slotWidth/2, routeBounds[row].getCentreY());
                    }
                }
                else endPoint[row].setCentrePosition(routeBounds[row].getCentre());
            }
        }
        
        void resized() override {
            int w { getWidth()/getNumColumns() }, boxWidth { static_cast<int>(float(w) * 0.75f) };
            for (auto& i : effects) i.setSize(boxWidth, boxWidth);
            for (auto f : fx) for(auto i : f) i.iPtr->setSize(w,w);
            layOutItems();
        }
        
        void paint(Graphics& g) override {
            const float width { static_cast<float>(getWidth()) }, height { static_cast<float>(getHeight()) }, arrowLength { static_cast<float>(margin) }, w { static_cast<float>(endPoint[0].getWidth()) };
            Path bigArrow;
            bigArrow.startNewSubPath(0.f, height/2.f);
            bigArrow.addArrow({ 0.f  - width * 0.25f, height/2.f, width + width * 0.25f, height/2.f }, height/3.f, height * 1.5f, width * 0.8f);
            bigArrow.closeSubPath();
            g.setColour(Colours::blue.darker().darker().darker().withAlpha(0.6f));
            g.fillPath(bigArrow);
            routePath.clear();
            routePath.startNewSubPath(0.f, height/2.f);
            routePath.lineTo(arrowLength * 0.25f, height/2.f);
            routePath.addArrow({ arrowLength * 0.25f, height/2.f, arrowLength, height/2.f }, 10.f, 40.f, arrowLength/2.f);
            routePath.closeSubPath();
            Point<float> leftSplit { routeBounds[0].toFloat().getRight() - w/2.f, height/2.f };
            Point<float> rightSplit { routeBounds[3].toFloat().getX() + w/2.f, height/2.f };
            routePath.startNewSubPath(arrowLength, height/2.f);
            routePath.lineTo(leftSplit);
            if(fx[1].size() == 1 && fx[2].size() == 1) routePath.lineTo(rightSplit);
            else if (fx[1].size() > 1) {
                routePath.lineTo(fx[1][0]->getBounds().getCentre().toFloat());
                routePath.lineTo(fx[1][fx[1].size()-2]->getBounds().getCentre().toFloat());
                routePath.lineTo(rightSplit);
            }
            if(fx[2].size() > 1) {
                routePath.startNewSubPath(leftSplit);
                routePath.lineTo(fx[2][0]->getBounds().getCentre().toFloat());
                routePath.lineTo(fx[2][fx[2].size()-2]->getBounds().getCentre().toFloat());
                routePath.lineTo(rightSplit);
            }
            routePath.lineTo(width - arrowLength, height/2.f);
            routePath.addArrow({ width - arrowLength, height/2.f, width - arrowLength * 0.25f, height/2.f }, 10.f, 40.f, arrowLength/2.f);
            routePath.closeSubPath();
            routePath.startNewSubPath(width - arrowLength * 0.25f, height/2.f);
            routePath.lineTo(width, height/2.f);
            g.setColour(Colours::blue.darker());
            g.strokePath(routePath, PathStrokeType(5.f));
        }
        void startDragging (const var& desc, Component* comp) { DragAndDropContainer::startDragging(desc, comp); }
        
        LeftEmpiricalLAF lelaf;
        RightEmpiricalLAF relaf;
        Effect effects[numFX] { { *this, getFxID(0), hub.mgmt.apvts }, { *this, getFxID(1), hub.mgmt.apvts }, { *this, getFxID(2), hub.mgmt.apvts }, { *this, getFxID(3), hub.mgmt.apvts }, { *this, getFxID(4), hub.mgmt.apvts }, { *this, getFxID(5), hub.mgmt.apvts }, { *this, getFxID(6), hub.mgmt.apvts }, { *this, getFxID(7), hub.mgmt.apvts }, { *this, getFxID(8), hub.mgmt.apvts }, { *this, getFxID(9), hub.mgmt.apvts } };
        InsertionPoint endPoint[numRoutes] { { *this, 0 }, { *this, 1 }, { *this, 2 }, { *this, 3 } };
        Array<EffectRoutingState> fx[numRoutes];
        Rectangle<int> routeBounds[numRoutes];
        const int margin { 30 };
        Path routePath;
        enum { filterALow = 0, filterAHigh, filterBLow, filterBHigh, numSliders };
        Frame frames[numSliders + 2];
        EmpiricalSlider knobs[numSliders] { { true, true }, { false, true }, { true, true }, { false, true } };
        APVTS::SliderAttachment attachments[numSliders] {
            { hub.mgmt.apvts, { "filterALow" },      knobs[0] },
            { hub.mgmt.apvts, { "filterAHigh" },     knobs[1] },
            { hub.mgmt.apvts, { "filterBLow" },      knobs[2] },
            { hub.mgmt.apvts, { "filterBHigh" },     knobs[3] }
        };
        SadLabel labels[2] { { "FILTER A", false, false, 0.f }, { "FILTER B", false, false, 0.f } };
    };
    
//    struct RoutingMatrix : DragAndDropContainer, DeviantScreen {
//
//        struct Icon : public Component {
//            static constexpr int iconResolution { 50 };
//            Icon(const String eID) : effectID(eID) {
//                setInterceptsMouseClicks(false, false);
//                if(effectID.contains("static")) {
//                    Wave<float>::fillTable(wave, iconResolution, Wave<float>::sine);
//                    for (auto& i : wave) { i *= -0.3f; i += 0.5f; }
//                }
//                else if (effectID.contains("dynamic")) {
//                    Wave<float>::fillTable(wave, iconResolution, Wave<float>::sine);
//                    float offset { 0.7f }, step { -1.4f / float(iconResolution) };
//                    for (int i { 0 }; i < iconResolution; ++i, offset += step) wave[i] += offset;
//                    for (auto& i : wave) { i *= -0.3f; i += 0.5f; }
//                }
//                path.preallocateSpace(iconResolution * 3 + 100);
//                rectangle.preallocateSpace(40);
//            }
//            void paint(Graphics& g) {
//                auto bounds { getLocalBounds().toFloat() };
//                float w { bounds.getWidth() }, h { bounds.getHeight() };
//
//                path.clear();
//                rectangle.clear();
//
//                if(effectID.contains("filter")) {
//                    path.startNewSubPath(w * 0.1f, h);
//                    path.lineTo(w * (0.15f), h * 0.4f);
//                    path.quadraticTo(w * (0.15f), h * 0.3f, w * (0.25f), h * 0.3f);
//                    path.lineTo(w * (0.75f), h * 0.3f);
//                    path.quadraticTo(w * (0.85f), h * 0.3f, w * (0.85f), h * 0.4f);
//                    path.lineTo(w * 0.9f, h);
//                    path.lineTo(w * 0.9f, h);
//                    g.setColour(Colours::steelblue.darker().withAlpha(0.4f));
//                    g.fillPath(path);
//                    g.setColour(Colours::steelblue);
//                    g.strokePath(path, PathStrokeType(3.f));
//                    g.setColour(Colours::white.darker());
//                    rectangle.addRoundedRectangle(bounds, 10);
//                    g.strokePath(rectangle, PathStrokeType(3.f));
//                }
//                else {
//                    path.startNewSubPath(0.f, wave[0] * h);
//                    for (int i = 1; i < iconResolution; ++i) path.lineTo(float(i)/float(iconResolution) * w, wave[i] * h);
//                    g.setColour(Colours::steelblue);
//                    g.strokePath(path, PathStrokeType(3.f));
//                    g.setColour(Colours::white.darker());
//                    rectangle.addRoundedRectangle(bounds, 10);
//                    g.strokePath(rectangle, PathStrokeType(3.f));
//                }
//            }
//            String effectID;
//            float wave[iconResolution]{};
//            Path path, rectangle;
//        };
//
//        struct Effect;
//
//        struct InsertionPoint : Component, DragAndDropTarget {
//            InsertionPoint(RoutingMatrix& m, int r = 0, Effect* e = nullptr, Colour c = Colours::blue.withAlpha(0.f)) : matrix(m), route(r), effect(e), colour(c) {}
//            bool isInterestedInDragSource (const SourceDetails& /*dragSourceDetails*/) override { return true; }
//            void itemDragEnter (const SourceDetails& /*dragSourceDetails*/) override;
//            void itemDragExit (const SourceDetails& /*dragSourceDetails*/) override;
//            void itemDropped (const SourceDetails& details) override;
//            void paint (Graphics& g) override { g.fillAll(colour); }
//            void setTransparency(float alpha) { colour = Colours::blue.withAlpha(alpha); }
//            RoutingMatrix& matrix;
//            int route;
//            Effect* effect { nullptr };
//            Colour colour;
//        };
//
//        struct Effect : Component {
//            Effect(RoutingMatrix& r, const String eID, APVTS& apvts) :
//            effectID(eID), matrix(r), insertionPoint(matrix, 0, this), enabledAttachment(apvts, translate(eID + "Enabled"), enabledSlider), routeAttachment(apvts, translate(eID + "Route"), routeSlider), indexAttachment(apvts, translate(eID + "Index"), indexSlider), blendAttachment(apvts, translate(eID + "Blend"), blendSlider), icon(eID) {
//                addAndMakeVisible(icon);
//                setAlpha((bool)enabledSlider.getValue() ? 1.f : 0.5f);
//            }
//            void resized() override {
//                auto w { jmin(getWidth() - 15, getHeight() - 15) };
//                icon.setBounds(Rectangle<int>(w,w).withCentre(getLocalBounds().getCentre()));
//            }
//            String getID() { return effectID; }
//            void mouseDrag (const MouseEvent& e) override;
//            void mouseDown(const MouseEvent& e) override;
//            void mouseUp(const MouseEvent& e) override;
//            void setSVG    (const char* svg, const int svgSize) { d.loadPathFromData(svg, (size_t) svgSize); }
//
//            const String effectID;
//            RoutingMatrix& matrix;
//            InsertionPoint insertionPoint;
//            Slider enabledSlider, routeSlider, indexSlider, blendSlider;
//            APVTS::SliderAttachment enabledAttachment, routeAttachment, indexAttachment, blendAttachment;
//            hi_res::time_point t1 = {}, t2 = {};
//            Icon icon;
//            Path d;
//        };
//
//        struct EffectRoutingState {
//            bool operator<(const EffectRoutingState&) const;
//            explicit operator bool() const { return ePtr != nullptr; }
//            Effect* operator->() { return ePtr; }
//            Effect* get() { return ePtr; }
//            Effect& operator*() { return *ePtr; }
//            Effect* ePtr; InsertionPoint* iPtr; bool enabled{}; int route{}; int index{}; double blend{};
//        };
//
//        enum { preRoute, upperRoute, lowerRoute, postRoute, numRoutes };
//
//        RoutingMatrix(APVTS& s, UndoManager* uM) : DeviantScreen(s, uM) {
//
////            for (int i { 0 }; i < numFX; ++i) addAndMakeVisible(effects[i]);
////            for (int i { 0 }; i < numSliders; ++i) {
////                knobs[i].setLookAndFeel(&relaf);
////                knobs[i].setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
////                if(i % 2 == 0) {
////                    knobs[i].setRotaryParameters(0.0f, 5.81333f, true);
////                }
////                else {
////                    knobs[i].setRotaryParameters(5.81333f, 0.0f, true);
////                }
////                addAllAndMakeVisible(*this, knobs[i], frames[i]);
////            }
////            addAllAndMakeVisible(*this, frames[numSliders], frames[numSliders + 1], labels[0], labels[1]);
////            addAllAndMakeVisible(*this, knobs, frames, labels);
//
//            EffectRoutingState tempFX[numFX];
//            for (int i { 0 }; i < numFX; ++i) {
//                tempFX[i] = { &effects[i], &effects[i].insertionPoint, static_cast<bool>(effects[i].enabledSlider.getValue()), static_cast<int>(effects[i].routeSlider.getValue()), static_cast<int>(effects[i].indexSlider.getValue()), effects[i].blendSlider.getValue() };
//            }
//            std::sort(tempFX, tempFX + numFX);
//            for (int i { 0 }; i < numFX; ++i) fx[tempFX[i].route].add(tempFX[i]);
//            for (int j { 0 }; j < numRoutes; ++j) { fx[j].add({ nullptr, &endPoint[j] }); for(auto f : fx[j]) addAndMakeVisible(f.iPtr); }
//
//            for (int i { 0 }; i < numFX; ++i) addAndMakeVisible(effects[i]);
//
////            knobs[1].onValueChange = [&,this]() {
////                if (knobs[0].getValue() > knobs[1].getValue()) knobs[0].setValue(knobs[1].getValue(), sendNotificationSync); };
////            knobs[0].onValueChange = [&,this]() {
////                if (knobs[1].getValue() < knobs[0].getValue()) knobs[1].setValue(knobs[0].getValue(), sendNotificationSync); };
////            knobs[3].onValueChange = [&,this]() {
////                if (knobs[2].getValue() > knobs[3].getValue()) knobs[2].setValue(knobs[3].getValue(), sendNotificationSync); };
////            knobs[2].onValueChange = [&,this]() {
////                if (knobs[3].getValue() < knobs[2].getValue()) knobs[3].setValue(knobs[2].getValue(), sendNotificationSync); };
//        }
//        void updateWaveTable() override {}
//        void init() override {}
//
//        int getNumInMiddle() { return jmax(1, fx[1].size()-1, fx[2].size()-1); }
//        int getNumColumns() { return getNumInMiddle() + jmax(1, fx[0].size()-1) + jmax(1, fx[3].size()-1); }
//        void moveEffect(Effect* effectComponent, int newRow, int newIndex) {
//            const int oldRow { static_cast<int>(effectComponent->routeSlider.getValue()) }, oldIndex { static_cast<int>(effectComponent->indexSlider.getValue()) };
//
//            fx[newRow].insert(newIndex, fx[oldRow].removeAndReturn(oldIndex));
//
//            int w { getWidth()/getNumColumns() }, boxWidth { static_cast<int>(float(w) * 0.75f) };
//
//            if(oldRow != newRow) {
//                for (auto j : fx) for(auto f : j) f.iPtr->setSize(w,w);
//                for (auto& e : effects) e.setSize(boxWidth, boxWidth);
//            }
//            for (int row = 0; row < numRoutes; ++row) {
//                for (int i { 0 }; i < fx[row].size()-1; ++i) {
//                    if (fx[row][i].index != i) { String iString { fx[row][i]->effectID + "Index" }; apvts.getParameter(iString)->beginChangeGesture(); fx[row][i]->indexSlider.setValue(i); }
//                    if (fx[row][i].route != row) { String rString { fx[row][i]->effectID + "Route" }; apvts.getParameter(rString)->beginChangeGesture(); fx[row][i]->routeSlider.setValue(row); }
//                }
//            }
//            for (int row = 0; row < numRoutes; ++row) {
//                for (int i { 0 }; i < fx[row].size()-1; ++i) {
//                    if (fx[row][i].index != i) { String iString { fx[row][i]->effectID + "Index" }; apvts.getParameter(iString)->endChangeGesture(); fx[row].getReference(i).index = i;  }
//                    if (fx[row][i].route != row) { String rString { fx[row][i]->effectID + "Route" }; apvts.getParameter(rString)->endChangeGesture(); fx[row].getReference(i).route = row;  }
//                }
//            }
//            layOutItems();
//        }
//
//        void layOutItems() {
//            auto w { 2 * getHeight()/12 }, h { getHeight()/8 };
//            for (int i { 0 }; i < numSliders; ++i)  {
//                frames[i].setSize(w,h);
//                frames[i].setCentrePosition(i%2 * w * 2 + w/2, i < 2 ? h/2 : getHeight() - h/2);
//                knobs[i].setBounds(frames[i].getBounds());
//                knobs[i].setTextBoxStyle(knobs[i].isLeft ? Slider::TextBoxLeft : Slider::TextBoxRight, false, h, h);
//            }
//            frames[numSliders].setBounds(knobs[0].getBounds().getRight(), knobs[0].getY(), w, h);
//            frames[numSliders + 1].setBounds(knobs[2].getBounds().getRight(), knobs[2].getY(), w, h);
//            labels[0].setBounds(frames[numSliders].getBounds());
//            labels[1].setBounds(frames[numSliders + 1].getBounds());
//            routeBounds[upperRoute] = getLocalBounds().reduced(margin, 0);
//            routeBounds[preRoute]   = routeBounds[upperRoute].removeFromLeft(getWidth()/getNumColumns() * jmax(fx[0].size()-1, 1));
//            routeBounds[postRoute]  = routeBounds[upperRoute].removeFromRight(getWidth()/getNumColumns() * jmax(fx[3].size()-1, 1));
//            routeBounds[lowerRoute] = routeBounds[upperRoute].removeFromBottom(getHeight()/2);
//
//            for (int slotWidth, position, row = 0; row < numRoutes; ++row) {
//                if(fx[row].size() > 1) {
//                    slotWidth = routeBounds[row].getWidth() / (fx[row].size() - 1), position = routeBounds[row].getX() + slotWidth/2;
//                    for (int i { 0 }; i < fx[row].size(); ++i, position += slotWidth) {
//                        if(fx[row][i]) fx[row][i]->setCentrePosition(position, routeBounds[row].getCentreY());
//                        fx[row][i].iPtr->setCentrePosition(position - slotWidth/2, routeBounds[row].getCentreY());
//                    }
//                }
//                else endPoint[row].setCentrePosition(routeBounds[row].getCentre());
//            }
//        }
//
//        void resized() override {
//            int w { getWidth()/getNumColumns() }, boxWidth { static_cast<int>(float(w) * 0.75f) };
//            for (auto& i : effects) i.setSize(boxWidth, boxWidth);
//            for (auto f : fx) for(auto i : f) i.iPtr->setSize(w,w);
//            layOutItems();
//        }
//
//        void paint(Graphics& g) override {
//            const float width { static_cast<float>(getWidth()) }, height { static_cast<float>(getHeight()) }, arrowLength { static_cast<float>(margin) }, w { static_cast<float>(endPoint[0].getWidth()) };
//            routePath.clear();
//            routePath.startNewSubPath(0.f, height/2.f);
//            routePath.lineTo(arrowLength * 0.25f, height/2.f);
//            routePath.addArrow({ arrowLength * 0.25f, height/2.f, arrowLength, height/2.f }, 10.f, 40.f, arrowLength/2.f);
//            routePath.closeSubPath();
//            Point<float> leftSplit { routeBounds[0].toFloat().getRight() - w/2.f, height/2.f };
//            Point<float> rightSplit { routeBounds[3].toFloat().getX() + w/2.f, height/2.f };
//            routePath.startNewSubPath(arrowLength, height/2.f);
//            routePath.lineTo(leftSplit);
//            if(fx[1].size() == 1 && fx[2].size() == 1) routePath.lineTo(rightSplit);
//            else if (fx[1].size() > 1) {
//                routePath.lineTo(fx[1][0]->getBounds().getCentre().toFloat());
//                routePath.lineTo(fx[1][fx[1].size()-2]->getBounds().getCentre().toFloat());
//                routePath.lineTo(rightSplit);
//            }
//            if(fx[2].size() > 1) {
//                routePath.startNewSubPath(leftSplit);
//                routePath.lineTo(fx[2][0]->getBounds().getCentre().toFloat());
//                routePath.lineTo(fx[2][fx[2].size()-2]->getBounds().getCentre().toFloat());
//                routePath.lineTo(rightSplit);
//            }
//            routePath.lineTo(width - arrowLength, height/2.f);
//            routePath.addArrow({ width - arrowLength, height/2.f, width - arrowLength * 0.25f, height/2.f }, 10.f, 40.f, arrowLength/2.f);
//            routePath.closeSubPath();
//            routePath.startNewSubPath(width - arrowLength * 0.25f, height/2.f);
//            routePath.lineTo(width, height/2.f);
//            g.setColour(Colours::blue.darker());
//            g.strokePath(routePath, PathStrokeType(5.f));
//        }
//        void startDragging (const var& desc, Component* comp) { DragAndDropContainer::startDragging(desc, comp); }
//
//        LeftEmpiricalLAF lelaf;
//        RightEmpiricalLAF relaf;
//        Effect effects[numFX] { { *this, getFxID(0), apvts }, { *this, getFxID(1), apvts }, { *this, getFxID(2), apvts }, { *this, getFxID(3), apvts }, { *this, getFxID(4), apvts }, { *this, getFxID(5), apvts }, { *this, getFxID(6), apvts }, { *this, getFxID(7), apvts }, { *this, getFxID(8), apvts }, { *this, getFxID(9), apvts } };
//        InsertionPoint endPoint[numRoutes] { { *this, 0 }, { *this, 1 }, { *this, 2 }, { *this, 3 } };
//        Array<EffectRoutingState> fx[numRoutes];
//        Rectangle<int> routeBounds[numRoutes];
//        const int margin { 30 };
//        Path routePath;
//        enum { filterALow = 0, filterAHigh, filterBLow, filterBHigh, numSliders };
//        Frame frames[numSliders + 2];
//        EmpiricalSlider knobs[numSliders] { { true, true }, { false, true }, { true, true }, { false, true } };
//        APVTS::SliderAttachment attachments[numSliders] {
//            { apvts, { "filterALow" },      knobs[0] },
//            { apvts, { "filterAHigh" },     knobs[1] },
//            { apvts, { "filterBLow" },      knobs[2] },
//            { apvts, { "filterBHigh" },     knobs[3] }
//        };
//        SadLabel labels[2] { { "FILTER A", false, false, 0.f }, { "FILTER B", false, false, 0.f } };
//    };
    
}
