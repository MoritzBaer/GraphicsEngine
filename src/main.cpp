#include "Game.h"

int main() {

  Engine::WindowManager::Init();

  Game game("Test Project");

  while (game.IsRunning())
    game.CalculateFrame();

  Engine::WindowManager::Cleanup();

  return 0;
}