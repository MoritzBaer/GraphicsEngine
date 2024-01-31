#include "Engine/Graphics/HelloTriangleApp.h"
#include "Engine/Maths/Matrix.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>

void print_mat_4(float* m) {
    for(int i = 0; i < 16; i++) {
            std::cout << m[i] << "\n";
    }
}

void compare_matrices(float* m1, float* m2, const char * name1, const char* name2, int rows, int columns) {
    std::cout << name1 << " | " << name2 << "\n";
    for(int i = 0; i < rows * columns; i++) {
        std::cout << m1[i] << " | " << m2[i] << "\n";
    }
}

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