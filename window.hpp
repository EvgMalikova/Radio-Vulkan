#ifndef WINDOW
#define WINDOW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <config.h>
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;



class WindowBase
	{
 public:
	  
	  WindowBase(){m_framebufferResized=false;};
	  ~WindowBase(){};
	   
	 
	  void CleanUp();
	  void InitWindow();
	  GLFWwindow* GetWindow(){return m_window;};
	  void SetFameBuffeResized(bool val){
	    m_framebufferResized = val;
	  }
	  
	  
 private:	  
	  GLFWwindow* m_window;
	  bool m_framebufferResized;
	  
	};


#ifdef LOG_All
#define DEBUG_LOG std::cout

#else
class log_disabled_output {};
static log_disabled_output log_disabled_output_instance;

template<typename T>
log_disabled_output& operator << (log_disabled_output& any, T const& thing) { return any; }

// std::endl simple, quick and dirty
log_disabled_output& operator << (log_disabled_output& any, std::ostream&(*)(std::ostream&)) { return any; }

#  define DEBUG_LOG log_disabled_output_instance 
#endif

#ifdef LOG_All
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


#endif // 