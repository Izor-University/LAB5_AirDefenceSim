#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

#include "../physics/IPhysicalObject.hpp"
#include "../entities/Radar.hpp"
#include "../entities/Target.hpp"
#include "../entities/Obstacle.hpp"
#include "../systems/SimulationWorld.hpp"

class SfmlVisualizer {
private:
    sf::RenderTarget& RenderTarget;
    float Scale;
    sf::Vector2f CameraOffset;

public:
    SfmlVisualizer(sf::RenderTarget& target, float Scale);

    sf::Vector2f WorldToScreen(const Vector3D& WorldPos) const;
    Vector3D ScreenToWorld(const sf::Vector2f& ScreenPos) const;

    void Render(const std::vector<std::shared_ptr<IPhysicalObject>>& Objects,
                const std::vector<std::shared_ptr<Radar>>& Radars,
                const std::vector<RaySegment>& Rays,
                const std::vector<FusedTrack>& Tracks,   // ИЗМЕНЕНО
                int SelectedTrackId);                    // ДОБАВЛЕНО
};