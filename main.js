/**
 *
 */
'use strict';

// TODO: Search for "TODO" and do it

var gl = null;
const camera = {
  rotation: {
    x: 0,
    y: 0
  }
};

//scene graph nodes
var root = null;
var rotateLight;
var rotateNode;

//textures
var renderTargetColorTexture;
var renderTargetDepthTexture;
var floorTexture;

//framebuffer variables
var renderTargetFramebuffer;
var framebufferWidth = 512;
var framebufferHeight = 512;

//load the required resources using a utility function
loadResources({
  vs: 'shader/texture.vs.glsl',
  fs: 'shader/texture.fs.glsl',
  vs_debug_light: 'shader/debugLight.vs.glsl',
  fs_debug_light: 'shader/debugLight.fs.glsl',
  model_pond_stone1: 'models/pond/stone1.obj',
  model_pond_stone2: 'models/pond/stone2.obj',
  model_pond_stone3: 'models/pond/stone3.obj',
  model_pond_stone4: 'models/pond/stone4.obj',
  model_pond_stone5: 'models/pond/stone5.obj'
}).then(function (resources /*an object containing our keys with the loaded resources*/) {
  init(resources);

  render(0);
});

function init(resources) {
  //create a GL context
  gl = createContext();

  gl.enable(gl.DEPTH_TEST);

  //create scenegraph
  root = createSceneGraph(gl, resources);
  root.append(createPond(gl, resources));

  initInteraction(gl.canvas);
}

function createSceneGraph(gl, resources) {
  //create scenegraph
  const root = new ShaderSGNode(createProgram(gl, resources.vs, resources.fs));

  //light debug helper function
  function createLightSphere() {
    return new ShaderSGNode(createProgram(gl, resources.vs_debug_light, resources.fs_debug_light), [
      new RenderSGNode(makeSphere(.2,10,10))
    ]);
  }

  //initialize light
  let light = new LightSGNode(); //use now framework implementation of light node
  light.ambient = [0.2, 0.2, 0.2, 1];
  light.diffuse = [0.8, 0.8, 0.8, 1];
  light.specular = [1, 1, 1, 1];
  light.position = [0, 0, 0];

  rotateLight = new TransformationSGNode(mat4.create());
  let translateLight = new TransformationSGNode(glm.translate(0,2,2)); //translating the light is the same as setting the light position

  rotateLight.append(translateLight);
  translateLight.append(light);
  translateLight.append(createLightSphere()); //add sphere for debugging: since we use 0,0,0 as our light position the sphere is at the same position as the light source
  root.append(rotateLight);

  return root;
}

function createPond(gl, resources) {
  const pondScene = new ShaderSGNode(createProgram(gl, resources.vs, resources.fs));

  // initializing the different stones
  let stone1 = new MaterialSGNode([
    new RenderSGNode(resources.model_pond_stone1)
  ]);
  let stone2 = new MaterialSGNode([
    new RenderSGNode(resources.model_pond_stone2)
  ]);
  let stone3 = new MaterialSGNode([
    new RenderSGNode(resources.model_pond_stone3)
  ]);
  let stone4 = new MaterialSGNode([
    new RenderSGNode(resources.model_pond_stone4)
  ]);
  let stone5 = new MaterialSGNode([
    new RenderSGNode(resources.model_pond_stone5)
  ]);

  // TODO: apply texture instead of simple material
  stone1.ambient = [0.5, 0.5, 0.5, 1.];
  stone1.diffuse = [0.6, 0.6, 0.6, 1.];
  stone1.specular = [1., 1., 1., 1.];
  stone1.shininess = 50.;
  stone2.ambient = stone1.ambient;
  stone2.diffuse = stone1.diffuse;
  stone2.specular = stone1.specular;
  stone2.shininess = stone1.shininess;
  stone3.ambient = stone1.ambient;
  stone3.diffuse = stone1.diffuse;
  stone3.specular = stone1.specular;
  stone3.shininess = stone1.shininess;
  stone4.ambient = stone1.ambient;
  stone4.diffuse = stone1.diffuse;
  stone4.specular = stone1.specular;
  stone4.shininess = stone1.shininess;
  stone5.ambient = stone1.ambient;
  stone5.diffuse = stone1.diffuse;
  stone5.specular = stone1.specular;
  stone5.shininess = stone1.shininess;

  // set stones
  pondScene.append(new TransformationSGNode(glm.transform({ translate: [0., 0., 0.], rotateY: 0, scale: 0.2 }), [
      stone1
  ]));
  pondScene.append(new TransformationSGNode(glm.transform({ translate: [1., 0., 0.], rotateY: 0, scale: 0.2 }), [
      stone2
  ]));
  pondScene.append(new TransformationSGNode(glm.transform({ translate: [2., 0., 0.], rotateY: 0, scale: 0.2 }), [
      stone3
  ]));
  pondScene.append(new TransformationSGNode(glm.transform({ translate: [3., 0., 0.], rotateY: 0, scale: 0.2 }), [
      stone4
  ]));
  pondScene.append(new TransformationSGNode(glm.transform({ translate: [4., 0., 0.], rotateY: 0, scale: 0.2 }), [
      stone5
  ]));

  return pondScene;
}


function render(timeInMilliseconds) {
  checkForWindowResize(gl);

  //setup viewport
  gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
  gl.clearColor(0.9, 0.9, 0.9, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  //setup context and camera matrices
  const context = createSGContext(gl);
  context.projectionMatrix = mat4.perspective(mat4.create(), convertDegreeToRadians(30), gl.drawingBufferWidth / gl.drawingBufferHeight, 0.01, 100);
  //very primitive camera implementation
  let lookAtMatrix = mat4.lookAt(mat4.create(), [0,4,-8], [0,0,0], [0,1,0]);
  let mouseRotateMatrix = mat4.multiply(mat4.create(),
                          glm.rotateX(camera.rotation.y),
                          glm.rotateY(camera.rotation.x));
  context.viewMatrix = mat4.multiply(mat4.create(), lookAtMatrix, mouseRotateMatrix);

  //update animations
  context.timeInMilliseconds = timeInMilliseconds;

  rotateLight.matrix = glm.rotateY(timeInMilliseconds*0.05);

  //render scenegraph
  root.render(context);

  //animate
  requestAnimationFrame(render);
}

//camera control
function initInteraction(canvas) {
  const mouse = {
    pos: { x : 0, y : 0},
    leftButtonDown: false
  };
  function toPos(event) {
    //convert to local coordinates
    const rect = canvas.getBoundingClientRect();
    return {
      x: event.clientX - rect.left,
      y: event.clientY - rect.top
    };
  }
  canvas.addEventListener('mousedown', function(event) {
    mouse.pos = toPos(event);
    mouse.leftButtonDown = event.button === 0;
  });
  canvas.addEventListener('mousemove', function(event) {
    const pos = toPos(event);
    const delta = { x : mouse.pos.x - pos.x, y: mouse.pos.y - pos.y };
    if (mouse.leftButtonDown) {
      //add the relative movement of the mouse to the rotation variables
  		camera.rotation.x += delta.x;
  		camera.rotation.y += delta.y;
    }
    mouse.pos = pos;
  });
  canvas.addEventListener('mouseup', function(event) {
    mouse.pos = toPos(event);
    mouse.leftButtonDown = false;
  });
  //register globally
  document.addEventListener('keypress', function(event) {
    //https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent
    if (event.code === 'KeyR') {
      camera.rotation.x = 0;
  		camera.rotation.y = 0;
    }
  });
}

function convertDegreeToRadians(degree) {
  return degree * Math.PI / 180
}
