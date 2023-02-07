#include "controller.h"
#include "sampleApp.h"

CameraController::CameraController(){
   mouse_observer.bind_callback(
     [this](const MouseEvent& m) {
         this->mouse_handler(m);
     });
   key_observer.bind_callback(
     [this](const KeyEvent& k) {
         this->key_handler(k);
     });
   
   std::cout<<"Events for camera were bound "<<std::endl;
 }
 
 void CameraController::mouse_handler(const MouseEvent& m){
   //std::cout<<"EVENT mouse"<<std::endl;
   if(active)
   {  
     float scale = (ideal_mspf/mspf); 
     float local_rot = rotate_speed*scale;
     switch(m.eid)
     {
       case EventId::MOUSE_DOWN:
       case EventId::MOUSE_UP:
         mouse_x = m.x;
         mouse_y = m.y;
       break;
       case EventId::MOUSE_DRAG:
       {
         if(m.button & int(MouseButtonId::LEFT))
         {
           int motion_x = m.x - mouse_x;
           int motion_y = m.y - mouse_y;
           float x_rot = 0, y_rot = 0;
           if(motion_x || motion_y)
           {
             x_rot = -motion_x*local_rot; 
             y_rot = -motion_y*local_rot;
             if(m.button>>8 & int(KeyModifierId::SHIFT))
             {
               camera.RotateTargetAround(x_rot, y_rot, 0.f);
               
             }
              
             else
             {
               camera.RotateAroundTarget(x_rot, -y_rot, 0.f);
               
               //camera.rotate(x_rot*2.5, -y_rot*2.5);
              
                           
               //std::cout<<"Camera rotation"<<std::endl;
             }
               
 
             mouse_x = m.x;
             mouse_y = m.y;
             modified = true;
             
           }         
         }
       }
       break;
 
       default:
       break;
     }
   }
 }
 
 void CameraController::key_handler(const KeyEvent& k){
   //std::cout<<"EVENT key"<<std::endl;
   // Scale for smooth movement with varying fps
   if(active)
   {
     float scale = (ideal_mspf/mspf); 
     float distance = move_speed*scale;
 
     if(k.eid == EventId::KEY_DOWN)
     {
       if(k.mod & int(KeyModifierId::SHIFT))
       {
 
         if     (k.key == 'w')  camera.MoveCamAndTargetForward(distance);
         else if(k.key == 'd')  camera.MoveCamAndTargetRight(distance);
         else if(k.key == 'q')  camera.MoveCamAndTargetUpward(distance);
         else if(k.key == 's')  camera.MoveCamAndTargetForward(-distance);
         else if(k.key == 'a')  camera.MoveCamAndTargetRight(-distance);
         else if(k.key == 'e')  camera.MoveCamAndTargetUpward(-distance);     
       }
       else
       {
         if     (k.key == 'w')  camera.MoveForward(distance);
         else if(k.key == 'd')  camera.MoveRight(distance);
         else if(k.key == 'q')  camera.MoveUpward(distance);
         else if(k.key == 's')  camera.MoveForward(-distance);
         else if(k.key == 'a')  camera.MoveRight(-distance);
         else if(k.key == 'e')  camera.MoveUpward(-distance);  
       }
       modified = true;
     }
   }
 }
 
 void CameraController::init(glm::vec3& pos, glm::vec3& at, glm::vec3& up, float m, float r, float imspf){
   camera.Create(pos, at, up);
   move_speed = m;
   rotate_speed = r;
   ideal_mspf = imspf;
 }
 
 void CameraController::reposition(glm::vec3&& pos, glm::vec3&& at, glm::vec3&& up)
 {
   camera.Create(pos, at, up);
 }

  ClientController::ClientController(SplotchServer* server){
   
    observer.bind_callback(
      [this](const ClientEvent& c) {
          this->handler(c);
          
      });
    std::cout<<"Event was bound"<<std::endl;
    if(server) s = server;
    else std::cerr<<"Client controller recieved NULL server pointer\n"<<std::endl;
  }

  void ClientController::handler(const ClientEvent& c)
  {
    switch(c.eid)
    {
      // When a client connects to the server
      // We  send a resize event automatically, followed by the most recent image
      case EventId::IMG_CONNECT:
       printf("Image server client connected\n");
       //s->spawn_viewresize();
       s->image_modified = true;
       break;
      case EventId::IMG_DISCONNECT:
        printf("Image server client disconnected\n");
      break;
      case EventId::CMD_CONNECT:
       // When a client connects to the server
      // We  send a resize event automatically, followed by the most recent image
       printf("Control server client connected\n");
       //s->notify_interface(InterfaceType::USER);
       s->image_modified = true;
       break;
      case EventId::CMD_DISCONNECT:
        printf("Control server client disconnected\n");
      break;
    default:
      printf("No event\n");
      break;   
    }  
  }

  // Client support is experimental, default client hardcoded here. 
  // Full client support reads this client data from an input file.
  void ClientController::load_default_client()
  {
    clients[""] = ClientData();
    /*clients[""].files = { 
                          "/Users/tims/Splotch/web/splotch-server/splotch/default.par",
                         // "/Users/tims/Splotch/web/splotch-server/splotch/snap092.par", 
                          "/Users/tims/Splotch/web/splotch-server/splotch/hdf5.par",
                          "/Users/tims/Splotch/web/splotch-server/splotch/millenium.par",
                          "/Users/tims/Splotch/web/splotch-server/splotch/mill-cut.par"
                        };

    clients[""].download_id = 0;

    clients[""].colourmaps = {
                                "palettes/Blue.pal",
                                "palettes/BlueSimple.pal",
                                "palettes/CyanPurple.pal",
                                "palettes/Fancy.pal",
                                "palettes/Klaus.pal",
                                "palettes/LongFancy.pal",
                                "palettes/M51.pal",
                                "palettes/M51_stars.pal",
                                "palettes/NewSplotch.pal",
                                "palettes/OldSplotch.pal",
                                "palettes/Orion.pal",
                                "palettes/OrionNew1.pal",
                                "palettes/RedBlue.pal",
                                "palettes/RedSimple.pal",
                                "palettes/Stars.pal",
                                "palettes/Tipsy.pal",
                                "palettes/TronInv.pal",
                                "palettes/Yellow.pal",
                            };*/
  }

  ClientController::ClientData& ClientController::client_data(std::string s)
  {
    if(clients.find(s) == clients.end())
      return clients[""];
    else return clients[s];
  }
