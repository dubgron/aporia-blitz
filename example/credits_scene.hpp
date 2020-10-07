#pragma once

#include <scene.hpp>

class CreditsScene : public Aporia::Scene
{
    using SceneTransition = std::function<void(int)>;

public:
    CreditsScene(Aporia::Logger& logger, const SceneTransition& exit, Aporia::Sprite credits)
        : Aporia::Scene(logger, "credits"), exit(exit), credits(std::move(credits))
    {

    }

    void on_load() override
    {
    }

    void on_input(const Aporia::InputManager& inputs) override
    {
        if (inputs.is_any_key_triggered())
            exit(-1);
    }

    void on_update(Aporia::Deltatime dt) override
    {
    }

    void on_draw(Aporia::Renderer& renderer) override
    {
        renderer.draw(credits);
    }

    void on_unload() override
    {
    }

private:
    SceneTransition exit;
    Aporia::Sprite credits;
};
