//
//  SSAmbientOcclusion.hpp
//  acg_im
//
//  Created by Jan on 19.01.17.
//  Copyright © 2017 Jan. All rights reserved.
//

#ifndef SSAmbientOcclusion_hpp
#define SSAmbientOcclusion_hpp

#include <stdio.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

class SSAmbienOcclusion{
    GLuint _depthBufferHandle;
    GLuint _positionBufferHandle;
    GLuint _normalBufferHandle;
    GLuint _colorBufferHandle;
    GLuint deferredFBO;
    std::vector<glm::vec3> _ssaoKernel;
    GLuint _noiseTexture;
    
    GLuint _ssaoFBO;
    GLuint _ssaoBlurFBO;
    GLuint _ssaoColorBuffer;
    GLuint _ssaoColorBufferBlur;
    
    int _screenWidth;
    int _screenHeight;
    

public:
    SSAmbienOcclusion(int screenWidth, int screenHeight) : _screenWidth{screenWidth}, _screenHeight{screenHeight}{};
    void createDepthBuffer();
    void createPositionBuffer();
    void createNormalBuffer();
    void createColorBuffer();
    void setAttachements();
    void createNoiseTexture();
    GLfloat interp(GLfloat a, GLfloat b, GLfloat f);
    
    void createSSAOFBO();
    
    
    
    
    void renderScreenQuad();
    
    GLuint getDepthBufferHandle();
    GLuint getPositionBufferHandle();
    GLuint getNormalBufferHandle();
    GLuint getColorBufferHandle();
    std::vector<glm::vec3> getSSAOKernel();
    GLuint getNoiseTexture();
    
    GLuint getSSAOFBO();
    GLuint getSSAOBlurFBO();
    GLuint getSSAOColorBuffer();
    GLuint getSSAOColorBufferBlur();
    
    ~SSAmbienOcclusion();

};

#endif /* SSAmbientOcclusion_hpp */
