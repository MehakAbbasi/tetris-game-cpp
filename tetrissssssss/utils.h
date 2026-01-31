#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <algorithm>

const char title[] = "PF-Project Tetris";
const int M = 22; // Height adjusted for the red frame
const int N = 10;
extern int holdPiece;
extern bool canHold;
extern int holdColor;

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float lifetime;
};
extern std::vector<Particle> particles;

void spawnParticles(int row, int colorIdx);
void updateParticles(float dt);
// Declarations (The "Map") - Use 'extern' so they can be shared across files
extern int gameGrid[M][N]; 
extern int point_1[4][2], point_2[4][2];
extern int shadow_point[4][2];

extern int score;
extern int level;
extern int linesCleared;
extern bool gameOver;
extern int colorNum;
extern int nextColorNum;
extern int lockedRows;
extern sf::Clock levelClock;

extern int highScores[10];

enum GameState { MENU, GAME, HIGHSCORE, HELP, PAUSED };
extern GameState currentState;