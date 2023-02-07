#ifndef SPLOTCH_CONTROLLER_H
#define SPLOTCH_CONTROLLER_H

#include "server/camera.h"
#include "server/event.h"

#include <vulkan/vulkan.h>
#include <iostream>

class SplotchServer;


class CameraController{
public:
  CameraController();
  
  /* The following operations provide an access to camera (SimpCamera) vulkan related staff from the controller*/
  
  void SetWindowSize(int width, int height){
      camera.SetWindowSize(width,height);
  }
  void SetModelScale(float model_scale)
  {
      camera.SetModelScale(model_scale);
  }
  
  void SetDevice(VkDevice dev)
  {
      camera.SetDevice(dev);
  }
  
  Camera2 GetCamera()
  {
      return camera;
  }
  
  
  void CreateUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, int size){
      camera.CreateUniformBuffers(device, physicalDevice, size);
  }
  void UpdateUniformBuffer(VkDevice device, uint32_t currentImage, float model_scale, int width, int height, float rotate_x, float rotate_y)
  {
      camera.UpdateUniformBuffer(device, currentImage, model_scale, width, height, rotate_x, rotate_y);
      
  }
  

  
 /*Conventional for camera controller*/
  
  void init(glm::vec3& pos, glm::vec3& at, glm::vec3& up, float m, float r, float imspf);

  void reposition(glm::vec3&& pos, glm::vec3&& at, glm::vec3&& up);

  void mouse_handler(const MouseEvent& m);

  void key_handler(const KeyEvent& k);

  // Passthroughs for camera info
  // Not necessarily normalized
  glm::vec3 eye(){return camera.GetCameraPosition();}
  glm::vec3 target(){return camera.GetTarget();}
  glm::vec3 up(){return camera.GetUpVector();}


  Observer<MouseEvent>  mouse_observer;
  Observer<KeyEvent>    key_observer;
  float mspf          = 100;
  bool modified       = false;
  float move_speed    = 0.1;
  float rotate_speed  = 0.1;
  bool active         = false;
  void SetCam (SimpCamera& camera) {
      cam=camera;
  }
  
private: 
  // Camera
  Camera2 camera;
  SimpCamera cam;
  float ideal_mspf  = 32; 
  float mouse_x     = 0;
  float mouse_y     = 0;
  
  /*int m_width;
  int m_height;
  VkDevice m_device;
  float m_model_scale;*/
};

//
// Handles client interaction
// Temporarily holds a pointer to the server
// Will be replaced by placing actions in internal action queue
//
class ClientController
{
public: 
struct ClientData {
    int download_id;
    std::vector<std::string> files;
    std::vector<std::string> colourmaps;
  };
  ClientController(SplotchServer* server);
  void handler(const ClientEvent& c);
  void load_default_client();
  ClientData& client_data(std::string s);
  bool exists(std::string s) {
    return clients.count(s);
  } 
  ClientData& retrieve(std::string name){
    return clients[name];
  }

  Observer<ClientEvent>  observer;

private: 
  SplotchServer* s;
  std::map<std::string, ClientData> clients;
};

#endif