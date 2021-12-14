#pragma once
#include "../../Source/sadistic.h"
#include "shaders.h"

namespace sadistic {
    
    using namespace juce::gl;

//    enum { wetSignal = 0, drySignal = 1, numSignals };
    static constexpr int scopeSize { SCOPESIZE };
    
    struct ScopeRenderer :  public Component, public juce::OpenGLRenderer {
        ScopeRenderer(APVTS& state, ScopeBuffer(& wf)[numSignals]) : apvts(state), scopeBuffer(wf) {
            mapSamplesToVertices();
            openGLContext.setOpenGLVersionRequired (OpenGLContext::OpenGLVersion::openGL3_2);
            openGLContext.setRenderer(this);
            openGLContext.attachTo(*this);
        }

        void start() { openGLContext.setContinuousRepainting (true); }
        void stop() { openGLContext.setContinuousRepainting (false); }

        void newOpenGLContextCreated() override {
            rmsShader = std::make_unique<OpenGLShaderProgram> (openGLContext);
            if (rmsShader->addVertexShader (rmsVertexShader) && rmsShader->addFragmentShader (rmsFragmentShader) && rmsShader->link()) {
                rmsShader->use();
                for (auto& u : rUniforms.data) {
                    u.id = openGLContext.extensions.glGetUniformLocation (rmsShader->getProgramID(), u.key.toRawUTF8());
                    u.ptr = std::make_unique<OpenGLShaderProgram::Uniform>(*rmsShader, u.key.toRawUTF8());
                }
            }
            waveShader = std::make_unique<OpenGLShaderProgram> (openGLContext);
            if (waveShader->addVertexShader (waveVertexShader)
                && waveShader->addShader (waveGeometryShader, 0x8DD9) && waveShader->addFragmentShader (waveFragmentShader)
                && waveShader->link()) {
                waveShader->use();
                for (auto& u : wUniforms.data) {
                    u.id = openGLContext.extensions.glGetUniformLocation (waveShader->getProgramID(), u.key.toRawUTF8());
                    u.ptr = std::make_unique<OpenGLShaderProgram::Uniform>(*waveShader, u.key.toRawUTF8());
                }
            }
            
            for (GLuint signal = 0; signal < numSignals; ++signal) {
                openGLContext.extensions.glGenBuffers (1, &waveVBO[signal]);
                openGLContext.extensions.glGenBuffers (1, &rmsVBO[signal]);
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, rmsVBO[signal]);
                float vertexes[12] = { 1.0f,   1.0f,  0.0f, 1.0f,  -1.0f,  0.0f, -1.0f, -1.0f,  0.0f, -1.0f,  1.0f,  0.0f };
                openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STREAM_DRAW);
                openGLContext.extensions.glGenBuffers (1, &rmsEBO[signal]);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, rmsEBO[signal]);
                GLuint indexes[6] = { 0, 1, 3, 1, 2, 3 };
                openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STREAM_DRAW);
            }
        }

        void openGLContextClosing() override {}
        void renderOpenGL() override {
            jassert (OpenGLHelpers::isContextActive());
            const float renderingScale = (float) openGLContext.getRenderingScale();
            glViewport (0, 0, roundToInt (renderingScale * getWidth()), roundToInt (renderingScale * getHeight()));
            OpenGLHelpers::clear (Colours::black);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            
            float blend = static_cast<float>(*apvts.getRawParameterValue("blend"))/100.f;
            for (GLuint signal = 0; signal < numSignals; ++signal) {
                getNewScopeData(signal);
                if(!approximatelyEqual(waveData[signal][0], 0.f)) {
                    if(signal == drySignal) blend = 1.f - blend;
                    Colour c = colour[signal];
                    
                    waveShader->use();
                    wUniforms["colour"]->set(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());
                    wUniforms["blend"]->set(blend);
                    
                    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, waveVBO[signal]);
                    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(vertices[signal]), vertices[signal], GL_STREAM_DRAW);
                    openGLContext.extensions.glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)nullptr);
                    openGLContext.extensions.glEnableVertexAttribArray (0);
                    glDrawArrays(GL_TRIANGLES, 0, 3 * (scopeSize-1));
                
                    rmsShader->use();
                    rUniforms["colour"]->set(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());
                    rUniforms["blend"]->set(blend);
                    rUniforms["waveData"]->set(waveData[signal], scopeSize);
                    rUniforms["resolution"]->set((float) renderingScale * getWidth(), (float) renderingScale * getHeight());

                    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, rmsVBO[signal]);
                    openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, rmsEBO[signal]);
                    openGLContext.extensions.glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)nullptr);
                    openGLContext.extensions.glEnableVertexAttribArray (0);
                    glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
                }
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
            }
        }
        
        void mapSamplesToVertices() {
            const auto xIncrement { 2.f / (scopeSize-1) };
            for (int signal = 0; signal < numSignals; ++signal) {
                float posIter { -1.f };
                for (int i = 0; i < (scopeSize-1); ++i) {
                    vertices[signal][i*3].position[0] = posIter;
                    posIter += xIncrement;
                    vertices[signal][i*3+2].position[0] = posIter;
                }
            }
        }
        
        void getNewScopeData(GLuint signal) {
            const auto frame { scopeBuffer[signal].getFrameToRead() };
            if (frame) {
                
                //set wave vertex data
                for (GLuint i = 0; i < (scopeSize-1); ++i) {
                    vertices[signal][i*3].position[1] = frame[i]/2.f;
                    vertices[signal][i*3+2].position[1] = frame[(i+1)]/2.f;
                }
                
                //set rms vertex data
                for (GLuint i = 0; i < scopeSize; ++i) waveData[signal][i] = frame[i];
                scopeBuffer[signal].finishedRendering(frame);
            }
        }
        
        void reset() { for (size_t signal = 0; signal < numSignals; ++signal) reset(signal); }
        void reset(size_t signal) {
            for (size_t i = 0; i < (scopeSize-1); ++i) {
                vertices[signal][i*3].position[1] = 0.f;
                vertices[signal][i*3+2].position[1] = 0.f;
            }
            for (size_t i = 0; i < scopeSize; ++i) waveData[signal][i] = 0.f;
        }

    private:
        
        APVTS& apvts;
        OpenGLContext openGLContext;
        const float ampMult { 1.875f }, ampOffset { 0.9375f }, rmsHoldFactor { 1.04f };
        GLuint waveVAO[numSignals], waveVBO[numSignals], rmsVAO[numSignals], rmsVBO[numSignals], rmsEBO[numSignals];
        std::unique_ptr<OpenGLShaderProgram> waveShader, rmsShader;
        unique_ptr_map<String, OpenGLShaderProgram::Uniform> wUniforms { { "blend", "colour" } }, rUniforms { { "blend", "colour", "resolution", "waveData" } };
        struct Vertex { float position[4] { 0.f, 0.f, 0.f, 1.f }; };
        Vertex vertices[numSignals][(scopeSize-1)*3];
        float waveData[numSignals][scopeSize]{};
        Colour colour[numSignals] { Colour::fromFloatRGBA(0.f, 0.3f, 1.f, 1.f), Colour::fromFloatRGBA(1.f, 1.f, 0.2f, 1.f) };
        ScopeBuffer(& scopeBuffer)[numSignals];

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeRenderer)
    };
}

