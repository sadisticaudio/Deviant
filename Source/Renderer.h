#pragma once
#include "../../Source/sadistic.h"
#include "shaders.h"

namespace sadistic {
    
    using namespace juce::gl;
    
    struct ScopeRenderer :  public Component, public juce::OpenGLRenderer {
        
        static constexpr int scopeSize { SCOPESIZE };
        
        ScopeRenderer(APVTS& state, ScopeBuffer(& wf)[numSignals]) : apvts(state), scopeBuffer(wf) {
            mapSamplesToVertices();
            openGLContext.setOpenGLVersionRequired (OpenGLContext::OpenGLVersion::openGL3_2);
            openGLContext.setRenderer(this);
            openGLContext.attachTo(*this);
            openGLContext.setContinuousRepainting (true);
        }
        ~ScopeRenderer() override { openGLContext.detach(); }
        void getMatrix(float angle = 1.f) {
            const auto rotation { Matrix3D<float>::rotation({ MathConstants<float>::halfPi/4.f/angle, MathConstants<float>::halfPi/2.f, 0.f }) };
            const auto view { Matrix3D<float>(Vector3D<float>(0.0f, 0.0f, -10.0f)) }, viewMatrix { rotation * view };
            const auto scaleFactor { 4.f }, w { 1.f / scaleFactor }, h { w * 1.2f * getLocalBounds().toFloat().getAspectRatio (false) };
            const auto projectionMatrix { Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 20.0f) };
            matrix = viewMatrix * projectionMatrix; }
        void resized() override { getMatrix(); }
        
        void newOpenGLContextCreated() override {
            Image image { ImageFileFormat::loadFrom (Data::moonStrip_png, Data::moonStrip_pngSize) };
            moonTexture.loadImage(ImageFileFormat::loadFrom (Data::moonStrip_png, Data::moonStrip_pngSize));
            moonShader = std::make_unique<OpenGLShaderProgram> (openGLContext);
            if (moonShader->addVertexShader (moonVertexShader) &&
                moonShader->addShader (moonGeometryShader, 0x8DD9) &&
                moonShader->addFragmentShader (moonFragmentShader) &&
                moonShader->link()) {
                moonShader->use();
                for (auto& u : mUniforms.data) {
                    u.id = openGLContext.extensions.glGetUniformLocation (moonShader->getProgramID(), u.key.toRawUTF8());
                    u.ptr = std::make_unique<OpenGLShaderProgram::Uniform>(*moonShader, u.key.toRawUTF8());
                }
                
            }
            
            waveShader = std::make_unique<OpenGLShaderProgram> (openGLContext);
            if (waveShader->addVertexShader (waveVertexShader) &&
                waveShader->addShader (waveGeometryShader, 0x8DD9) &&
                waveShader->addFragmentShader (waveFragmentShader) &&
                waveShader->link()) {
                waveShader->use();
                for (auto& u : wUniforms.data) {
                    u.id = openGLContext.extensions.glGetUniformLocation (waveShader->getProgramID(), u.key.toRawUTF8());
                    u.ptr = std::make_unique<OpenGLShaderProgram::Uniform>(*waveShader, u.key.toRawUTF8());
                }
                
            }
            for (GLuint signal = 0; signal < numSignals; ++signal)  openGLContext.extensions.glGenBuffers (1, &vbo[signal]);
        }
        
        void openGLContextClosing() override { moonShader->release(); waveShader->release(); moonTexture.release(); }
        
        void renderOpenGL() override {
            jassert (OpenGLHelpers::isContextActive());
            const float renderingScale = (float) openGLContext.getRenderingScale();
            glViewport (0, 0, roundToInt (renderingScale * getWidth()), roundToInt (renderingScale * getHeight()));
            OpenGLHelpers::clear (Colours::black);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE);//_MINUS_SRC_ALPHA);
            openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
            glEnable (GL_TEXTURE_2D);
            
            float blend = powf(*apvts.getRawParameterValue("mainBlend"), 0.5f);
            float wtPosition = 0.f;
            
            moonTexture.bind();
            
            for (GLuint signal = 0; signal < numSignals; ++signal) {
                getNewScopeData(signal);
                if(signal == drySignal) { blend = 1.f - blend; wtPosition = 0.f; }
                Colour c = colour[signal];
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vbo[signal]);
                openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(vertices[signal]), vertices[signal], GL_STREAM_DRAW);
                openGLContext.extensions.glVertexAttribPointer (0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)nullptr);
                openGLContext.extensions.glEnableVertexAttribArray (0);
                
                waveShader->use();
                wUniforms["colour"]->set(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());
                wUniforms["blend"]->set(blend);
                wUniforms["wtPosition"]->set(wtPosition);
                wUniforms["matrix"]->setMatrix4 (matrix.mat, 1, false);
                
                glDrawArrays(GL_TRIANGLES, 0, 3 * (scopeSize-1));
                
                moonShader->use();
                mUniforms["colour"]->set(c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue());
                mUniforms["blend"]->set(blend);
                mUniforms["wtPosition"]->set(wtPosition);
                mUniforms["matrix"]->setMatrix4 (matrix.mat, 1, false);
                mUniforms["demoTexture"]->set((GLint) 0);
                
                glDrawArrays(GL_TRIANGLES, 0, 3 * (scopeSize-1));
                
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            moonTexture.unbind();
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
                rms[signal] = Wave<float>::getRMS(frame, scopeSize);
                for (GLuint i = 0; i < (scopeSize-1); ++i) {
                    vertices[signal][i*3].position[1] = frame[i]/2.f;
                    vertices[signal][i*3+2].position[1] = frame[(i+1)]/2.f;
                }
                scopeBuffer[signal].finishedRendering(frame);
            }
        }
        
    private:
        
        APVTS& apvts;
        OpenGLContext openGLContext;
        GLuint vbo[numSignals];
        std::unique_ptr<OpenGLShaderProgram> moonShader, waveShader;
        unique_ptr_map<String, OpenGLShaderProgram::Uniform> mUniforms { { "blend", "colour", "wtPosition", "demoTexture", "matrix" } }, wUniforms { { "blend", "colour", "wtPosition", "matrix" } };
        struct Vertex { float position[4] { 0.f, 0.f, 0.f, 1.f }; };
        Vertex vertices[numSignals][(scopeSize-1)*3];
        OpenGLTexture moonTexture;
        float rms[numSignals]{};
        Colour colour[numSignals] { Colour::fromFloatRGBA(0.f, 1.f, 1.f, 1.f), Colour::fromFloatRGBA(0.7f, 1.f, 0.2f, 1.f) };
        ScopeBuffer(& scopeBuffer)[numSignals];
        Matrix3D<float> matrix;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeRenderer)
    };
}

