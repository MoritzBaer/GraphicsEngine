#include "Engine/Math/Matrix.h"
#include "Engine/Graphics/HelloTriangleApp.h"

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