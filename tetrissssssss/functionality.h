#ifndef FUNCTIONALITY_H
#define FUNCTIONALITY_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include "utils.h"

// --- FORWARD DECLARATIONS ---
void checkGameOver();
void calculateShadow();
void loadHighScores();
void saveHighScore(int s);

// --- 1. THE COLLISION CHECKER ---
bool checkCollision() {
    for (int i = 0; i < 4; i++) {
        if (point_1[i][0] < 0 || point_1[i][0] >= N || point_1[i][1] >= M - 1) // Change M to M - 1
            return false;
        if (point_1[i][1] >= 0 && gameGrid[point_1[i][1]][point_1[i][0]])
            return false;
    }
    return true;
}

bool checkShadowCollision(int temp[4][2]) {
    for (int i = 0; i < 4; i++) {
        if (temp[i][0] < 0 || temp[i][0] >= N || temp[i][1] >= M - 1) // Change M to M - 1
            return false;
        if (temp[i][1] >= 0 && gameGrid[temp[i][1]][temp[i][0]]) return false;
    }
    return true;
}
// --- 2. HIGH SCORE LOGIC (Fixes 'loadHighScores' & 'saveHighScore' errors) ---
void loadHighScores() {
    std::ifstream file("highscores.txt");
    if (file.is_open()) {
        for (int i = 0; i < 10; i++) {
            if (!(file >> highScores[i])) highScores[i] = 0;
        }
        file.close();
    }
}

void saveHighScore(int s) {
    loadHighScores();
    if (s > highScores[9]) {
        highScores[9] = s;
        std::sort(highScores, highScores + 10, std::greater<int>());
        std::ofstream file("highscores.txt");
        for (int i = 0; i < 10; i++) file << highScores[i] << "\n";
        file.close();
    }
}

// --- 3. SHADOW LOGIC ---
void calculateShadow() {
    for (int i = 0; i < 4; i++) {
        shadow_point[i][0] = point_1[i][0];
        shadow_point[i][1] = point_1[i][1];
    }
    while (checkShadowCollision(shadow_point)) {
        for (int i = 0; i < 4; i++) shadow_point[i][1]++;
    }
    for (int i = 0; i < 4; i++) shadow_point[i][1]--;
}

// --- 4. MOVEMENT & ROTATION ---
void movePiece(int dx) {
    for (int i = 0; i < 4; i++) {
        point_2[i][0] = point_1[i][0];
        point_2[i][1] = point_1[i][1];
        point_1[i][0] += dx;
    }
    if (!checkCollision()) {
        for (int i = 0; i < 4; i++) point_1[i][0] = point_2[i][0];
    }
    calculateShadow();
}

void rotatePiece() {
    for (int i = 0; i < 4; i++) {
        point_2[i][0] = point_1[i][0];
        point_2[i][1] = point_1[i][1];
    }
    int px = point_1[1][0];
    int py = point_1[1][1];
    for (int i = 0; i < 4; i++) {
        int x = point_1[i][1] - py;
        int y = point_1[i][0] - px;
        point_1[i][0] = px - x;
        point_1[i][1] = py + y;
    }
    if (!checkCollision()) {
        for (int i = 0; i < 4; i++) {
            point_1[i][0] = point_2[i][0];
            point_1[i][1] = point_2[i][1];
        }
    }
    calculateShadow();
}

// --- 5. DIFFICULTY & SCORING (Fixes 'updateDifficulty' error) ---
void updateDifficulty() {
    if (levelClock.getElapsedTime().asSeconds() >= 300) {
        if (lockedRows < M - 1) lockedRows++;
        levelClock.restart();
    }
}

void clearLines() {
    int k = M - 1;
    int cleared = 0;
    for (int i = M - 1; i >= lockedRows; i--) {
        int count = 0;
        for (int j = 0; j < N; j++) {
            if (gameGrid[i][j]) count++;
            gameGrid[k][j] = gameGrid[i][j];
        }
        if (count == N) {
            spawnParticles(i, 1); // <--- ADD THIS LINE: It triggers the explosion before row is gone
            // ... rest of your clearing logic
        }
        if (count < N) k--;
        else cleared++;
    }
    if (cleared > 0) {
        if (cleared == 1) score += 10 * level;
        else if (cleared == 2) score += 30 * level;
        else if (cleared == 3) score += 60 * level;
        else if (cleared == 4) score += 100 * level;
        linesCleared += cleared;
        if (linesCleared >= 10) { level++; linesCleared = 0; }
    }
}

// --- 6. FALLING & HARD DROP (Fixes 'hardDrop' error) ---
void fallingPiece(float& timer, float& delay) {
    if (timer > delay) {
        for (int i = 0; i < 4; i++) {
            point_2[i][0] = point_1[i][0];
            point_2[i][1] = point_1[i][1];
            point_1[i][1]++;
        }
        if (!checkCollision()) {
            for (int i = 0; i < 4; i++)
                if (point_2[i][1] >= 0) gameGrid[point_2[i][1]][point_2[i][0]] = colorNum;

            colorNum = 1 + rand() % 7;
            int n = (level == 1) ? rand() % 4 : rand() % 7;
            for (int i = 0; i < 4; i++) {
                point_1[i][0] = BLOCKS[n][i] % 4;
                point_1[i][1] = BLOCKS[n][i] / 4;
            }
            canHold = true; // Allow the player to hold again because a new piece has started
            checkGameOver();
        }
        timer = 0;
    }
    calculateShadow();
}

void hardDrop() {
    for (int i = 0; i < 4; i++) {
        point_1[i][0] = shadow_point[i][0];
        point_1[i][1] = shadow_point[i][1];
    }
    for (int i = 0; i < 4; i++) {
        if (point_1[i][1] >= 0) gameGrid[point_1[i][1]][point_1[i][0]] = colorNum;
    }
    colorNum = 1 + rand() % 7;
    int n = (level == 1) ? rand() % 4 : rand() % 7;
    for (int i = 0; i < 4; i++) {
        point_1[i][0] = BLOCKS[n][i] % 4;
        point_1[i][1] = BLOCKS[n][i] / 4;
    }
    canHold = true; // Allow the player to hold again because a new piece has started
    checkGameOver();
}

void checkGameOver() {
    for (int j = 0; j < N; j++) {
        if (gameGrid[0][j]) {
            gameOver = true;
            saveHighScore(score);
        }
    }
}
// --- PARTICLE LOGIC ---
void spawnParticles(int row, int colorIdx) {
    sf::Color colors[] = { sf::Color::White, sf::Color::Cyan, sf::Color::Red, sf::Color::Green,
                          sf::Color::Blue, sf::Color::Magenta, sf::Color::Yellow, sf::Color(255, 165, 0) };
    for (int j = 0; j < N; j++) {
        for (int p = 0; p < 4; p++) {
            Particle prt;
            prt.pos = sf::Vector2f(74.0f + (float)j * 18, 62.0f + (float)row * 18);
            prt.vel = sf::Vector2f((float)(rand() % 100 - 50) / 10.0f, (float)(rand() % 100 - 50) / 10.0f);
            prt.color = colors[colorIdx % 8];
            prt.lifetime = 1.0f;
            particles.push_back(prt);
        }
    }
}

void updateParticles(float dt) {
    for (int i = 0; i < (int)particles.size(); ) {
        particles[i].pos += particles[i].vel;
        particles[i].vel.y += 0.2f; // Gravity
        particles[i].lifetime -= dt;
        if (particles[i].lifetime <= 0) particles.erase(particles.begin() + i);
        else i++;
    }
}
// --- 2. Hold Logic ---
void performHold() {
    if (!canHold) return;

    // Determine current shape index (simple reverse check of BLOCKS)
    int currentShape = 0; // You may need a global 'currentShapeIndex' to make this easier

    if (holdPiece == -1) {
        holdPiece = colorNum - 1; // Store current
        holdColor = colorNum;
        // Spawn new piece
        colorNum = 1 + rand() % 7;
        int n = rand() % 7;
        for (int i = 0; i < 4; i++) {
            point_1[i][0] = BLOCKS[n][i] % 4;
            point_1[i][1] = BLOCKS[n][i] / 4;
        }
    }
    else {
        int temp = holdPiece;
        holdPiece = colorNum - 1;
        int tempColor = holdColor;
        holdColor = colorNum;
        colorNum = tempColor;

        for (int i = 0; i < 4; i++) {
            point_1[i][0] = BLOCKS[temp][i] % 4;
            point_1[i][1] = BLOCKS[temp][i] / 4;
        }
    }
    canHold = false;
}
#endif