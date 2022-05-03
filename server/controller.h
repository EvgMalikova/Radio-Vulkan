#ifndef SPLOTCH_CONTROLLER_H
#define SPLOTCH_CONTROLLER_H

#include "server/camera.h"
#include "server/event.h"
#include <iostream>

class SplotchServer;


class CameraController{
public:
  CameraController();

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
private: 
  // Camera
  Camera2 camera;
  float ideal_mspf  = 32; 
  float mouse_x     = 0;
  float mouse_y     = 0;
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