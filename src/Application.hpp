#pragma once

#include "Component.hpp"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <entt/entt.hpp>

#include <map>

class Application {
public:
    void run();

private:
    void processEvents();

    void handleEventControllerDeviceAdded(const SDL_Event &event);
    void handleEventControllerDeviceRemoved(const SDL_Event &event);
    void handleEventKeyDown(const SDL_Event &event);
    void handleEventKeyUp(const SDL_Event &event);
    void handleEventControllerButtonDown(const SDL_Event &event);
    void handleEventControllerButtonUp(const SDL_Event &event);

    void fixedUpdate([[maybe_unused]] float fixedDeltaTime);
    void update([[maybe_unused]] float deltaTime);
    void render();

    void renderWalls();
    void renderBricks();
    void renderBalls();
    void renderPaddles();

    void respawnWalls();
    void respawnBricks();
    void respawnBalls();
    void respawnPaddles();

    void rumbleController(std::uint16_t low_frequency_rumble, std::uint16_t high_frequency_rumble, std::uint32_t duration_ms);

    enum class CollisionLocation { None, Top, Bottom, Left, Right };

    void updatePositions(float fixedDeltaTime);
    void checkCollisions();

    static CollisionLocation checkAABBCollision(const Component::Transform &transformA, const Component::Transform &transformB);

    SDL_Window *mWindow = nullptr;
    SDL_Renderer *mRenderer = nullptr;
    SDL_GameController *mGameController = nullptr;
    std::map<std::uint8_t, std::uint8_t> mGameControllerButtons;
    std::map<std::int32_t, std::int32_t> mKeyboardKeys;
    bool mRunning = true;
    bool mPaused = false;
    bool mGameOver = false;
    entt::registry mRegistry;
};