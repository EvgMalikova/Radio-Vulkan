#include <pipeline.hpp>
#include <window.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


#include <tclap/CmdLine.h>

#include "sampleApp.h"


int main(int argc, char** argv)
{
    TCLAP::CmdLine cmd("Command description message", ' ', "0.9");

    TCLAP::ValueArg<std::string> nameArg("f", "filename", "Datacube filename", true, "LVHIS027.na.icln.fits", "string");
    cmd.add(nameArg);
    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg.
    std::string FitsFileName = nameArg.getValue();

    SampleApp app;
    app.SetFileName(FitsFileName);
    app.SetPipelineType(pv2::PipelineTYPE::Rasterisation);
    app.SetMode(pv2::InteractionMode::Headless);
    
   
    

    try {
        
       
        //bool interact = true; //the headless rendering still needs to be fully set up
        int width = 1000;
        int height = 1000;
        
        app.SetUpAll(width,height);
        app.Finalise();
        
       
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


 

