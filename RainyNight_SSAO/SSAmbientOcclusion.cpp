//
//  SSAmbientOcclusion.cpp
//  acg_im
//
//  Created by Jan on 19.01.17.
//  Copyright © 2017 Jan. All rights reserved.
//

#include "SSAmbientOcclusion.hpp"
#include <glm/glm.hpp>



void SSAmbienOcclusion::createPositionBuffer(){
    glGenTextures(1, &_positionBufferHandle);
    glBindTexture(GL_TEXTURE_2D, _positionBufferHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _screenWidth, _screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _positionBufferHandle, 0);

}

void SSAmbienOcclusion::createColorBuffer(){
    glGenTextures(1, &_colorBufferHandle);
    glBindTexture(GL_TEXTURE_2D, _colorBufferHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _screenWidth, _screenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _colorBufferHandle, 0);
}

void SSAmbienOcclusion::createDepthBuffer(){
    glGenRenderbuffers(1, &_depthBufferHandle);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBufferHandle);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _screenWidth, _screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBufferHandle);
}

void SSAmbienOcclusion::createNormalBuffer(){
    glGenTextures(1, &_normalBufferHandle);
    glBindTexture(GL_TEXTURE_2D, _normalBufferHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _screenWidth, _screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _normalBufferHandle, 0);
}

void SSAmbienOcclusion::setAttachements(){
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
}

void SSAmbienOcclusion::createSSAOFBO(){
    glGenFramebuffers(1, &_ssaoFBO);  glGenFramebuffers(1, &_ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaoFBO);
    // - SSAO color buffer
    glGenTextures(1, &_ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, _ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _screenWidth, _screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ssaoColorBuffer, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaoBlurFBO);
    glGenTextures(1, &_ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, _ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _screenWidth, _screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _ssaoColorBufferBlur, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAmbienOcclusion::renderScreenQuad(){
    GLuint quadVAO = 0;
    GLuint quadVBO;
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

GLfloat SSAmbienOcclusion::interp(GLfloat a, GLfloat b, GLfloat f){
    return a + f * (b - a);

}

void SSAmbienOcclusion::createNoiseTexture(){
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    //std::vector<glm::vec3> ssaoKernel;
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0;
        
        // Scale samples s.t. they're more aligned to center of kernel
        scale = interp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        _ssaoKernel.push_back(sample);
    }
    
    // Noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
   // GLuint noiseTexture;
    glGenTextures(1, &_noiseTexture);
    glBindTexture(GL_TEXTURE_2D, _noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
}


GLuint SSAmbienOcclusion::getColorBufferHandle(){
    return _colorBufferHandle;
}

GLuint SSAmbienOcclusion::getDepthBufferHandle(){
    return _depthBufferHandle;
}

GLuint SSAmbienOcclusion::getNormalBufferHandle(){
    return _normalBufferHandle;
}

GLuint SSAmbienOcclusion::getPositionBufferHandle(){
    return _positionBufferHandle;
}

std::vector<glm::vec3> SSAmbienOcclusion::getSSAOKernel(){
    return _ssaoKernel;
}

GLuint SSAmbienOcclusion::getSSAOFBO(){
    return _ssaoFBO;
}

GLuint SSAmbienOcclusion::getSSAOBlurFBO(){
    return _ssaoBlurFBO;
}

GLuint SSAmbienOcclusion::getSSAOColorBuffer(){
    return _ssaoColorBuffer;
}

GLuint SSAmbienOcclusion::getSSAOColorBufferBlur(){
    return _ssaoColorBufferBlur;
}

GLuint SSAmbienOcclusion::getNoiseTexture(){
    return _noiseTexture;
}
