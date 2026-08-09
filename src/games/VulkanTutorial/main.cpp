#include "core/engine.h"
#include <iostream>
#include <cstdlib>

// struct VulkanTutorial {
//     std::string name;
// };

int main()
{
    try
    {
        longroad::Engine app;
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    
    return EXIT_SUCCESS;
}