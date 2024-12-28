#include "Game.h"

int main() {

  Engine::WindowManager::Init();

  Game game("Test Project");

  for (int i = 0; i < 200; i++)
    game.CalculateFrame();

  Engine::WindowManager::Cleanup();

  return 0;
}