#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include "utils.h"
#include "pieces.h"
#include "functionality.h"

using namespace sf;

// --- GLOBAL VARIABLE DEFINITIONS ---
int gameGrid[M][N] = { 0 };
int point_1[4][2], point_2[4][2];
int shadow_point[4][2];
int score = 0;
int level = 1;
int linesCleared = 0;
bool gameOver = false;
int colorNum = 1;
int nextColorNum = 1;
int lockedRows = 0;
sf::Clock levelClock;
int highScores[10] = { 0 };
GameState currentState = MENU;
int holdPiece = -1;
bool canHold = true;
int holdColor = 0;
std::vector<Particle> particles;
// ---------------------------------------------------------------

int main() {
    srand(static_cast<unsigned int>(time(0)));
    RenderWindow window(VideoMode(320, 480), "Tetris Project");

    Texture obj1, obj2, obj3;
    if (!obj1.loadFromFile("tiles.png") || !obj2.loadFromFile("background.png") || !obj3.loadFromFile("frame.png")) {
        return -1;
    }

    Sprite sprite(obj1), background(obj2), frame(obj3);

    Font font;
    if (!font.loadFromFile("arial.ttf")) return -1;
    Text ui;
    ui.setFont(font);

    float timer = 0.0f, delay = 0.3f;
    Clock clock;
    loadHighScores();

    // Initial Piece Setup
    colorNum = 1 + rand() % 7;
    int n = rand() % 7; // Changed to %7 for all shapes
    for (int i = 0; i < 4; i++) {
        point_1[i][0] = BLOCKS[n][i] % 4;
        point_1[i][1] = BLOCKS[n][i] / 4;
    }

    float offsetX = 74.0f;
    float offsetY = 62.0f;

    while (window.isOpen()) {
        float deltaTime = clock.getElapsedTime().asSeconds();
        clock.restart();
        if (currentState == GAME) timer += deltaTime;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();

            if (e.type == Event::KeyPressed) {
                if (currentState == MENU || currentState == PAUSED) {
                    if (e.key.code == Keyboard::Num1) {
                        for (int i = 0; i < M; i++) for (int j = 0; j < N; j++) gameGrid[i][j] = 0;
                        score = 0; level = 1; gameOver = false; holdPiece = -1;
                        currentState = GAME;
                    }
                    if (e.key.code == Keyboard::Num2) currentState = HIGHSCORE;
                    if (e.key.code == Keyboard::Num3) currentState = HELP;
                    if (e.key.code == Keyboard::Num4) window.close();
                    if (currentState == PAUSED && e.key.code == Keyboard::Num5) currentState = GAME;
                }
                else if (currentState == GAME) {
                    if (e.key.code == Keyboard::P) currentState = PAUSED;
                    if (e.key.code == Keyboard::Up) rotatePiece();
                    if (e.key.code == Keyboard::Left) movePiece(-1);
                    if (e.key.code == Keyboard::Right) movePiece(1);
                    if (e.key.code == Keyboard::Space) hardDrop();
                    if (e.key.code == Keyboard::C) performHold();
                }

                if (e.key.code == Keyboard::Escape) currentState = MENU;
            }
        }

        if (currentState == GAME) {
            if (Keyboard::isKeyPressed(Keyboard::Down)) delay = 0.05f;
            else delay = 0.3f - (static_cast<float>(level) - 1.0f) * 0.05f;

            fallingPiece(timer, delay);
            clearLines();
            updateDifficulty();
            updateParticles(deltaTime); // Update physics
        }

        window.clear(Color::Black);
        window.draw(background);

        // --- DRAW PARTICLES ---
        for (auto& p : particles) {
            RectangleShape prtShape(Vector2f(2, 2));
            prtShape.setPosition(p.pos);
            prtShape.setFillColor(p.color);
            window.draw(prtShape);
        }

        sf::Color lightPink(255, 182, 193);

        if (currentState == MENU) {
            ui.setCharacterSize(30); ui.setStyle(sf::Text::Bold);
            ui.setFillColor(sf::Color::Black); ui.setString("TETRIS");
            ui.setPosition(offsetX + 42, offsetY + 22); window.draw(ui);
            ui.setFillColor(lightPink); ui.setPosition(offsetX + 40, offsetY + 20); window.draw(ui);

            ui.setCharacterSize(20);
            ui.setString("1. Start Game\n\n2. High Scores\n\n3. Help\n\n4. Exit");
            ui.setFillColor(sf::Color::Black); ui.setPosition(offsetX + 27, offsetY + 102); window.draw(ui);
            ui.setFillColor(lightPink); ui.setPosition(offsetX + 25, offsetY + 100); window.draw(ui);
        }
        else if (currentState == HIGHSCORE) {
            ui.setCharacterSize(18);
            std::string scoreList = "TOP 10 SCORES\n\n";
            for (int i = 0; i < 10; i++) scoreList += std::to_string(i + 1) + ". " + std::to_string(highScores[i]) + "\n";
            ui.setString(scoreList);
            ui.setFillColor(sf::Color::Black); ui.setPosition(offsetX + 17, offsetY + 22); window.draw(ui);
            ui.setFillColor(lightPink); ui.setPosition(offsetX + 15, offsetY + 20); window.draw(ui);
        }
        else if (currentState == HELP) {
            ui.setCharacterSize(16);
            ui.setString("CONTROLS:\n\nLeft/Right: Move\nUp: Rotate\nDown: Fast\nSpace: Drop\nC: Hold\nP: Pause\n\nESC for Menu");
            ui.setFillColor(sf::Color::Black); ui.setPosition(offsetX + 17, offsetY + 52); window.draw(ui);
            ui.setFillColor(lightPink); ui.setPosition(offsetX + 15, offsetY + 50); window.draw(ui);
        }
        else if (currentState == PAUSED) {
            ui.setCharacterSize(28); ui.setStyle(sf::Text::Bold);
            ui.setString("PAUSED");
            ui.setFillColor(sf::Color::Black); ui.setPosition(offsetX + 42, offsetY + 22); window.draw(ui);
            ui.setFillColor(lightPink); ui.setPosition(offsetX + 40, offsetY + 20); window.draw(ui);

            ui.setCharacterSize(20);
            ui.setString("1. New Game\n\n2. High Scores\n\n3. Help\n\n4. Exit\n\n5. Continue");
            ui.setFillColor(sf::Color::Black); ui.setPosition(offsetX + 27, offsetY + 82); window.draw(ui);
            ui.setFillColor(lightPink); ui.setPosition(offsetX + 25, offsetY + 80); window.draw(ui);
        }
        else if (currentState == GAME) {
            // Shadow
            for (int i = 0; i < 4; i++) {
                sprite.setTextureRect(IntRect(0, 0, 18, 18)); sprite.setColor(Color(255, 255, 255, 70));
                sprite.setPosition(shadow_point[i][0] * 18.0f, shadow_point[i][1] * 18.0f);
                sprite.move(offsetX, offsetY); window.draw(sprite);
            }
            sprite.setColor(Color::White);

            // Grid
            for (int i = 0; i < M; i++)
                for (int j = 0; j < N; j++) {
                    if (!gameGrid[i][j]) continue;
                    sprite.setTextureRect(IntRect(gameGrid[i][j] * 18, 0, 18, 18));
                    sprite.setPosition(j * 18.0f, i * 18.0f);
                    sprite.move(offsetX, offsetY); window.draw(sprite);
                }

            // Current Piece
            for (int i = 0; i < 4; i++) {
                sprite.setTextureRect(IntRect(colorNum * 18, 0, 18, 18));
                sprite.setPosition(point_1[i][0] * 18.0f, point_1[i][1] * 18.0f);
                sprite.move(offsetX, offsetY); window.draw(sprite);
            }

            // --- DRAW HOLD BOX ---
           // --- DRAW HOLD BOX (Shifted Left) ---
            ui.setCharacterSize(16);
            ui.setStyle(sf::Text::Bold);
            ui.setFillColor(sf::Color::Yellow);
            ui.setString("HOLD");

            // Changed from 15 to 5 to move it further left away from the frame
            ui.setPosition(5, 100);
            window.draw(ui);

            if (holdPiece != -1) {
                for (int i = 0; i < 4; i++) {
                    sprite.setTextureRect(IntRect(holdColor * 18, 0, 18, 18));
                    sprite.setScale(0.7f, 0.7f);

                    // Changed X from 10 to 2 to shift the preview blocks left
                    // Changed Y from 130 to 125 to tuck it closer under the "HOLD" text
                    sprite.setPosition(2 + (BLOCKS[holdPiece][i] % 4) * 14, 125 + (BLOCKS[holdPiece][i] / 4) * 14);
                    window.draw(sprite);
                }
                sprite.setScale(1.0f, 1.0f);
            }

            // --- HUD ---
            ui.setCharacterSize(20);
            ui.setFillColor(sf::Color::Black);
            ui.setString("SCORE: " + std::to_string(score)); ui.setPosition(21, 11); window.draw(ui);
            ui.setString("LEVEL: " + std::to_string(level)); ui.setPosition(21, 36); window.draw(ui);

            ui.setFillColor(sf::Color::Yellow);
            ui.setPosition(20, 10); window.draw(ui); // Draw Score
            ui.setPosition(20, 35); ui.setString("LEVEL: " + std::to_string(level)); window.draw(ui);

            ui.setStyle(sf::Text::Regular);
        }

        frame.setPosition(offsetX - 26, offsetY - 13);
        window.draw(frame);
        window.display();

        if (gameOver) { currentState = MENU; saveHighScore(score); }
    }
    return 0;
}