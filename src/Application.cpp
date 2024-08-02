#include "Application.hpp"

#include <glm/glm.hpp>

void Application::run() {
    if (SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1") == SDL_FALSE) {
        SDL_Log("Failed to set hint: %s", SDL_GetError());
    }

    if (SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1") == SDL_FALSE) {
        SDL_Log("Failed to set hint: %s", SDL_GetError());
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    mWindow = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!mWindow) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!mRenderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        std::exit(EXIT_FAILURE);
    }

    respawnGoal();
    respawnWalls();
    respawnBricks();
    respawnBalls();
    respawnPaddles();

    const float fixedDeltaTime = 1.0f / 240.0f;
    float accumulator = 0.0f;
    std::uint64_t lastFrameTime = SDL_GetTicks64();

    while (mRunning) {
        processEvents();

        std::uint64_t currentFrameTime = SDL_GetTicks64();
        float deltaTime = static_cast<float>(currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        if (!mPaused) {
            accumulator += deltaTime;
            while (accumulator > fixedDeltaTime) {
                fixedUpdate(fixedDeltaTime);
                accumulator -= fixedDeltaTime;
            }
            update(deltaTime);
        }
        render();
    }

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Application::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                mRunning = false;
                break;

            case SDL_CONTROLLERDEVICEADDED:
                handleEventControllerDeviceAdded(event);
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                handleEventControllerDeviceRemoved(event);
                break;

            case SDL_KEYDOWN:
                handleEventKeyDown(event);
                break;

            case SDL_KEYUP:
                handleEventKeyUp(event);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                handleEventControllerButtonDown(event);
                break;

            case SDL_CONTROLLERBUTTONUP:
                handleEventControllerButtonUp(event);
                break;

            default:
                break;
        }
    }
}

void Application::handleEventControllerDeviceAdded(const SDL_Event &event) {
    if (SDL_IsGameController(event.cdevice.which)) {
        mGameController = SDL_GameControllerOpen(event.cdevice.which);
        if (!mGameController) {
            SDL_Log("Failed to open game controller: %s", SDL_GetError());
            std::exit(EXIT_FAILURE);
        }
    }
}

void Application::handleEventControllerDeviceRemoved(const SDL_Event &event) {
    if (SDL_IsGameController(event.cdevice.which)) {
        SDL_GameControllerClose(mGameController);
        mGameController = nullptr;
    }
}

void Application::handleEventKeyDown(const SDL_Event &event) {
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        mPaused = !mPaused;
    }
    mKeyboardKeys[event.key.keysym.sym] = event.key.state;
}

void Application::handleEventKeyUp(const SDL_Event &event) {
    mKeyboardKeys[event.key.keysym.sym] = event.key.state;
}

void Application::handleEventControllerButtonDown(const SDL_Event &event) {
    if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
        mPaused = !mPaused;
    }
    mGameControllerButtons[event.cbutton.button] = event.cbutton.state;
}

void Application::handleEventControllerButtonUp(const SDL_Event &event) {
    mGameControllerButtons[event.cbutton.button] = event.cbutton.state;
}

void Application::fixedUpdate([[maybe_unused]] float fixedDeltaTime) {
    updatePositions(fixedDeltaTime);
    checkCollisions();
}

void Application::update([[maybe_unused]] float deltaTime) {
    if (!mGameOver) {
        bool flag = false;
        auto view = mRegistry.view<Component::Tag>();
        view.each([&](entt::entity, const Component::Tag &tag) {
            if (tag.name == "Brick") {
                flag = true;
            }
        });
        if (!flag) {
            mGameOver = !mGameOver;
        }
    } else {
        mGameOver = !mGameOver;
        respawnGoal();
        respawnWalls();
        respawnBricks();
        respawnBalls();
        respawnPaddles();
    }
}

void Application::render() {
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    SDL_RenderClear(mRenderer);

    renderGoal();
    renderWalls();
    renderBricks();
    renderBalls();
    renderPaddles();

    SDL_RenderPresent(mRenderer);
}

void Application::renderGoal() {
    auto view = mRegistry.view<Component::Tag, Component::Transform>();
    view.each([this](entt::entity, const Component::Tag &tag, const Component::Transform &transform) {
        if (tag.name == "Goal") {
            SDL_Rect rect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
            SDL_RenderFillRect(mRenderer, &rect);
        }
    });
}

void Application::renderWalls() {
    auto view = mRegistry.view<Component::Tag, Component::Transform>();
    view.each([this](entt::entity, const Component::Tag &tag, const Component::Transform &transform) {
        if (tag.name == "Wall") {
            SDL_Rect rect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
            SDL_RenderFillRect(mRenderer, &rect);
        }
    });
}

void Application::renderBricks() {
    auto view = mRegistry.view<Component::Tag, Component::Transform, Component::Sprite>();
    view.each([this](entt::entity, const Component::Tag &tag, const Component::Transform &transform, const Component::Sprite &sprite) {
        if (tag.name == "Brick") {
            SDL_Rect rect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, static_cast<std::uint8_t>(sprite.color[0]), static_cast<std::uint8_t>(sprite.color[1]), static_cast<std::uint8_t>(sprite.color[2]), static_cast<std::uint8_t>(sprite.color[3]));
            SDL_RenderFillRect(mRenderer, &rect);

            SDL_Rect outlineRect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, static_cast<std::uint8_t>(sprite.color[0]) * 0.8f, static_cast<std::uint8_t>(sprite.color[1]) * 0.8f, static_cast<std::uint8_t>(sprite.color[2]) * 0.8f, static_cast<std::uint8_t>(sprite.color[3]) * 0.8f);
            SDL_RenderDrawRect(mRenderer, &outlineRect);
        }
    });
}

void Application::renderBalls() {
    auto view = mRegistry.view<Component::Tag, Component::Transform, Component::Sprite>();
    view.each([this](entt::entity, const Component::Tag &tag, const Component::Transform &transform, const Component::Sprite &sprite) {
        if (tag.name == "Ball") {
            SDL_Rect rect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, static_cast<std::uint8_t>(sprite.color[0]), static_cast<std::uint8_t>(sprite.color[1]), static_cast<std::uint8_t>(sprite.color[2]), static_cast<std::uint8_t>(sprite.color[3]));
            SDL_RenderFillRect(mRenderer, &rect);

            SDL_Rect outlineRect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, static_cast<std::uint8_t>(sprite.color[0]) * 0.8f, static_cast<std::uint8_t>(sprite.color[1]) * 0.8f, static_cast<std::uint8_t>(sprite.color[2]) * 0.8f, static_cast<std::uint8_t>(sprite.color[3]));
            SDL_RenderDrawRect(mRenderer, &outlineRect);
        }
    });
}

void Application::renderPaddles() {
    auto view = mRegistry.view<Component::Tag, Component::Transform, Component::Sprite>();
    view.each([this](entt::entity, const Component::Tag &tag, const Component::Transform &transform, const Component::Sprite &sprite) {
        if (tag.name == "Paddle") {
            SDL_Rect rect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, static_cast<std::uint8_t>(sprite.color[0]), static_cast<std::uint8_t>(sprite.color[1]), static_cast<std::uint8_t>(sprite.color[2]), static_cast<std::uint8_t>(sprite.color[3]));
            SDL_RenderFillRect(mRenderer, &rect);

            SDL_Rect outlineRect{static_cast<int>(transform.position.x), static_cast<int>(transform.position.y), static_cast<int>(transform.scale.x), static_cast<int>(transform.scale.y)};
            SDL_SetRenderDrawColor(mRenderer, static_cast<std::uint8_t>(sprite.color[0]) * 0.8f, static_cast<std::uint8_t>(sprite.color[1]) * 0.8f, static_cast<std::uint8_t>(sprite.color[2]) * 0.8f, static_cast<std::uint8_t>(sprite.color[3]));
            SDL_RenderDrawRect(mRenderer, &outlineRect);
        }
    });
}

void Application::respawnGoal() {
    auto view = mRegistry.view<Component::Tag>();
    view.each([this](entt::entity entity, const Component::Tag &tag) {
        if (tag.name == "Goal") {
            mRegistry.destroy(entity);
        }
    });

    entt::entity entity2 = mRegistry.create();
    mRegistry.emplace<Component::Tag>(entity2, "Goal");
    mRegistry.emplace<Component::Transform>(entity2, glm::vec2(0.0f, 600.0f), glm::vec2(0.0f, 0.0f), glm::vec2(800.0f, 20.0f));
}

void Application::respawnWalls() {
    auto view = mRegistry.view<Component::Tag>();
    view.each([this](entt::entity entity, const Component::Tag &tag) {
        if (tag.name == "Wall") {
            mRegistry.destroy(entity);
        }
    });

    entt::entity entity1 = mRegistry.create();
    mRegistry.emplace<Component::Tag>(entity1, "Wall");
    mRegistry.emplace<Component::Transform>(entity1, glm::vec2(0.0f, -20.0f), glm::vec2(0.0f, 0.0f), glm::vec2(800.0f, 20.0f));

    entt::entity entity3 = mRegistry.create();
    mRegistry.emplace<Component::Tag>(entity3, "Wall");
    mRegistry.emplace<Component::Transform>(entity3, glm::vec2(-20.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 600.0f));

    entt::entity entity4 = mRegistry.create();
    mRegistry.emplace<Component::Tag>(entity4, "Wall");
    mRegistry.emplace<Component::Transform>(entity4, glm::vec2(800.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(20.0f, 600.0f));
}

void Application::respawnBricks() {
    const std::map<std::uint8_t, glm::vec4> colors{
            {0, glm::vec4{194, 57, 52, 255}},  // Red
            {1, glm::vec4{255, 167, 38, 255}}, // Orange
            {2, glm::vec4{255, 255, 100, 255}},// Yellow
            {3, glm::vec4{78, 188, 78, 255}},  // Green
            {4, glm::vec4{46, 116, 181, 255}}, // Blue
            {5, glm::vec4{216, 64, 185, 255}}, // Purple
            {6, glm::vec4{255, 105, 180, 255}},// Pink
            {7, glm::vec4{128, 128, 128, 255}} // Gray
    };

    const std::uint8_t rows = 8;
    const std::uint8_t cols = 10;

    auto view = mRegistry.view<Component::Tag>();
    view.each([this](entt::entity entity, const Component::Tag &tag) {
        if (tag.name == "Brick") {
            mRegistry.destroy(entity);
        }
    });

    for (std::uint8_t row = 0; row < rows; ++row) {
        for (std::uint8_t col = 0; col < cols; ++col) {
            entt::entity entity = mRegistry.create();
            mRegistry.emplace<Component::Tag>(entity, "Brick");
            mRegistry.emplace<Component::Transform>(entity, glm::vec2(static_cast<float>(col) * 80.0f, static_cast<float>(row) * 20.0f), glm::vec2(0.0f, 0.0f), glm::vec2(80.0f, 20.0f));
            mRegistry.emplace<Component::Sprite>(entity, colors.at(row));
        }
    }
}

void Application::respawnBalls() {
    auto view = mRegistry.view<Component::Tag>();
    view.each([this](entt::entity entity, const Component::Tag &tag) {
        if (tag.name == "Ball") {
            mRegistry.destroy(entity);
        }
    });

    entt::entity entity = mRegistry.create();
    mRegistry.emplace<Component::Tag>(entity, "Ball");
    mRegistry.emplace<Component::Transform>(entity, 0.5f * glm::vec2(800.0f - 10.0f, 600.0f - 10.0f), glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 10.0f));
    mRegistry.emplace<Component::Sprite>(entity, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f));
    mRegistry.emplace<Component::Movement>(entity, glm::vec2(200.0f, 200.0f));
}

void Application::respawnPaddles() {
    auto view = mRegistry.view<Component::Tag>();
    view.each([this](entt::entity entity, const Component::Tag &tag) {
        if (tag.name == "Paddle") {
            mRegistry.destroy(entity);
        }
    });

    entt::entity entity = mRegistry.create();
    mRegistry.emplace<Component::Tag>(entity, "Paddle");
    mRegistry.emplace<Component::Transform>(entity, glm::vec2(0.5f * (800.0f - 80.0f), 600.0f - 20.0f), glm::vec2(0.0f, 0.0f), glm::vec2(80.0f, 20.0f));
    mRegistry.emplace<Component::Sprite>(entity, glm::vec4(255.0f, 255.0f, 255.0f, 255.0f));
    mRegistry.emplace<Component::Movement>(entity, glm::vec2(400.0f, 0.0f));
}

void Application::rumbleController(std::uint16_t low_frequency_rumble, std::uint16_t high_frequency_rumble, std::uint32_t duration_ms) {
    if (mGameController) {
        SDL_GameControllerRumble(mGameController, low_frequency_rumble, high_frequency_rumble, duration_ms);
    }
}

void Application::updatePositions(float fixedDeltaTime) {
    auto ballView = mRegistry.view<Component::Tag, Component::Transform, Component::Movement>();
    ballView.each([&](entt::entity, const Component::Tag &tag, Component::Transform &ballTransform, Component::Movement &ballMovement) {
        if (tag.name == "Ball") {
            ballTransform.position.x += ballMovement.velocity.x * fixedDeltaTime;
            ballTransform.position.y += ballMovement.velocity.y * fixedDeltaTime;
        }
    });

    auto paddleView = mRegistry.view<Component::Tag, Component::Transform, Component::Movement>();
    paddleView.each([&](entt::entity, const Component::Tag &tag, Component::Transform &paddleTransform, Component::Movement &paddleMovement) {
        if (tag.name == "Paddle") {
            auto goalView = mRegistry.view<Component::Tag, Component::Transform>();
            goalView.each([&](entt::entity, const Component::Tag &tag, const Component::Transform &goalTransform) {
                if (tag.name == "Goal") {
                    if (paddleTransform.position.x > 0.0f) {
                        if (mKeyboardKeys[SDLK_LEFT]) {
                            paddleTransform.position.x -= paddleMovement.velocity.x * fixedDeltaTime;
                        }
                        if (mGameController && mGameControllerButtons[SDL_CONTROLLER_BUTTON_DPAD_LEFT]) {
                            paddleTransform.position.x -= paddleMovement.velocity.x * fixedDeltaTime;
                        }
                    }
                    if (paddleTransform.position.x + paddleTransform.scale.x < goalTransform.scale.x) {
                        if (mKeyboardKeys[SDLK_RIGHT]) {
                            paddleTransform.position.x += paddleMovement.velocity.x * fixedDeltaTime;
                        }
                        if (mGameController && mGameControllerButtons[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]) {
                            paddleTransform.position.x += paddleMovement.velocity.x * fixedDeltaTime;
                        }
                    }
                }
            });
        }
    });
}

void Application::checkCollisions() {
    auto ballView = mRegistry.view<Component::Tag, Component::Transform, Component::Movement>();
    ballView.each([&](entt::entity, const Component::Tag &tag, Component::Transform &ballTransform, Component::Movement &ballMovement) {
        if (tag.name == "Ball") {
            auto goalView = mRegistry.view<Component::Tag, Component::Transform>();
            goalView.each([&](entt::entity, const Component::Tag &tag, const Component::Transform &goalTransform) {
                if (tag.name == "Goal") {
                    switch (checkAABBCollision(ballTransform, goalTransform)) {
                        using enum CollisionLocation;
                        case Top:
                        case Bottom:
                            mGameOver = !mGameOver;
                            rumbleController(0xDEAD, 0xBEEF, 100);
                            ballMovement.velocity.y *= -1.0f;
                            break;
                        case Left:
                        case Right:
                            mGameOver = !mGameOver;
                            rumbleController(0xDEAD, 0xBEEF, 100);
                            ballMovement.velocity.x *= -1.0f;
                            break;
                        default:
                            break;
                    }
                }
            });

            auto wallView = mRegistry.view<Component::Tag, Component::Transform>();
            wallView.each([&](entt::entity, const Component::Tag &tag, const Component::Transform &wallTransform) {
                if (tag.name == "Wall") {
                    switch (checkAABBCollision(ballTransform, wallTransform)) {
                        using enum CollisionLocation;
                        case Top:
                        case Bottom:
                            rumbleController(0xDEAD, 0xBEEF, 100);
                            ballMovement.velocity.y *= -1.0f;
                            break;
                        case Left:
                        case Right:
                            rumbleController(0xDEAD, 0xBEEF, 100);
                            ballMovement.velocity.x *= -1.0f;
                            break;
                        default:
                            break;
                    }
                }
            });

            auto brickView = mRegistry.view<Component::Tag, Component::Transform>();
            brickView.each([&](entt::entity entity, const Component::Tag &tag, const Component::Transform &brickTransform) {
                if (tag.name == "Brick") {
                    switch (checkAABBCollision(ballTransform, brickTransform)) {
                        using enum CollisionLocation;
                        case Top:
                        case Bottom:
                            rumbleController(0xDEAD, 0xBEEF, 200);
                            mRegistry.destroy(entity);
                            ballMovement.velocity.y *= -1.0f;
                            break;
                        case Left:
                        case Right:
                            rumbleController(0xDEAD, 0xBEEF, 200);
                            mRegistry.destroy(entity);
                            ballMovement.velocity.x *= -1.0f;
                            break;
                        default:
                            break;
                    }
                }
            });

            auto paddleView = mRegistry.view<Component::Tag, Component::Transform>();
            paddleView.each([&](entt::entity, const Component::Tag &tag, const Component::Transform &paddleTransform) {
                if (tag.name == "Paddle") {
                    switch (checkAABBCollision(ballTransform, paddleTransform)) {
                        using enum CollisionLocation;
                        case Top:
                        case Bottom:
                            rumbleController(0xDEAD, 0xBEEF, 100);
                            ballMovement.velocity.y *= -1.0f;
                            break;
                        case Left:
                        case Right:
                            rumbleController(0xDEAD, 0xBEEF, 100);
                            ballMovement.velocity.x *= -1.0f;
                            break;
                        default:
                            break;
                    }
                }
            });
        }
    });
}

Application::CollisionLocation Application::checkAABBCollision(const Component::Transform &transformA, const Component::Transform &transformB) {
    const glm::vec2 minAPosition = transformA.position;
    const glm::vec2 minBPosition = transformB.position;

    const glm::vec2 maxAPosition = transformA.position + transformA.scale;
    const glm::vec2 maxBPosition = transformB.position + transformB.scale;

    using enum CollisionLocation;

    if (maxAPosition.x > minBPosition.x && minAPosition.x < maxBPosition.x && maxAPosition.y > minBPosition.y && minAPosition.y < maxBPosition.y) {
        const glm::vec2 overlap(glm::min(maxAPosition.x, maxBPosition.x) - glm::max(minAPosition.x, minBPosition.x), glm::min(maxAPosition.y, maxBPosition.y) - glm::max(minAPosition.y, minBPosition.y));

        if (const glm::vec2 ratio(overlap.x / transformA.scale.x, overlap.y / transformA.scale.y); ratio.x > ratio.y) {
            if (maxAPosition.y < maxBPosition.y) {
                return Top;
            } else {
                return Bottom;
            }
        } else {
            if (maxAPosition.x < maxBPosition.x) {
                return Left;
            } else {
                return Right;
            }
        }
    }

    return None;
}