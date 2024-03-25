// #include "Core/ECS.h"
// #include "Engine/AssetManager.h"
// #include "Engine/Debug/Logging.h"
// #include "Engine/Engine.h"
// #include "Engine/Graphics/Shader.h"
#include "Maths/Transformations.h"
#include <fstream>
#include <iomanip>
#include <iostream>

void printForMat(void *mat) {
  for (int i = 0; i < 16; i++) {
    float val = *(((float *)mat) + i);
    if (!std::signbit(val)) {
      std::cout << " ";
    }
    std::cout << val << " ";
  }
}

int main() {

  Engine::Maths::Vector3 axis{0.4f, 1.9f, 0.9f};
  auto mat0 = glm::rotate(2.0f, glm::vec3(0.4f, 1.9f, 0.9f));
  auto mat1 = Engine::Maths::Transformations::RodriguesRotation(axis, 2.0f);
  auto mat2 = Engine::Maths::Transformations::RotateAroundAxis(axis, 2.0f).RotationMatrix();

  std::cout << "GLM: \n";
  printForMat(&mat0);
  std::cout << "\nRodriguez: \n";
  printForMat(&mat1);
  std::cout << "\nQuaternion: \n";
  printForMat(&mat2);

  // Engine::Init("Test project");
  // Engine::RunMainLoop();
  // Engine::Cleanup();

  return 0;
}