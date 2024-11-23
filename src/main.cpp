#include "Game.h"

int main() {

  Game game("Test Project");

  for (int i = 0; i < 2000; i++)
    game.CalculateFrame();

  return 0;
}