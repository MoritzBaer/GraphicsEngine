#include "Engine/Graphics/HelloTriangleApp.h"
#include "Engine/Maths/Matrix.h"

#include <iostream>

int main() {
    HelloTriangleApp app;

    try {
        app.Run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    return 0;
}