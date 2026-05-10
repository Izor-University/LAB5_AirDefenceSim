#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "../physics/IPhysicalObject.hpp"
#include "../entities/Radar.hpp"
#include "../entities/Target.hpp"
#include "../entities/Obstacle.hpp"
#include "../systems/SimulationWorld.hpp"

class SfmlVisualizer {
private:
    sf::RenderWindow Window;
    float Scale;
    sf::Vector2f CameraOffset;

    sf::Font MainFont;
    sf::Text InfoText;

    sf::Vector2f WorldToScreen(const Vector3D& WorldPos) const;
    Vector3D ScreenToWorld(const sf::Vector2f& ScreenPos) const;

public:
    SfmlVisualizer(int Width, int Height, float Scale);

    bool IsOpen() const;
    std::optional<Vector3D> HandleEvents();

    // ОБНОВЛЕННАЯ СИГНАТУРА
    void Render(const std::vector<std::shared_ptr<IPhysicalObject>>& Objects,
                const std::vector<std::shared_ptr<Radar>>& Radars,
                const std::vector<RaySegment>& Rays,
                const std::vector<Vector3D>& Tracks, // Новый параметр (Координаты захваченных целей)
                const std::string& RadarStats);
};