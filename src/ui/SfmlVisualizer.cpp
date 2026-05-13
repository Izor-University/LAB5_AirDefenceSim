#include "../../include/ui/SfmlVisualizer.hpp"

SfmlVisualizer::SfmlVisualizer(sf::RenderTarget& target, float Scale)
    : RenderTarget(target), Scale(Scale),
      CameraOffset(RenderTarget.getSize().x / 2.0f, RenderTarget.getSize().y / 2.0f) {}

sf::Vector2f SfmlVisualizer::WorldToScreen(const Vector3D& WorldPos) const {
    return sf::Vector2f(CameraOffset.x + (WorldPos.X * Scale), CameraOffset.y - (WorldPos.Y * Scale));
}

Vector3D SfmlVisualizer::ScreenToWorld(const sf::Vector2f& ScreenPos) const {
    double X = (ScreenPos.x - CameraOffset.x) / Scale;
    double Y = (CameraOffset.y - ScreenPos.y) / Scale;
    return Vector3D(X, Y, 0.0);
}

void SfmlVisualizer::Render(const std::vector<std::shared_ptr<IPhysicalObject>>& Objects,
                            const std::vector<std::shared_ptr<Radar>>& Radars,
                            const std::vector<RaySegment>& Rays,
                            const std::vector<FusedTrack>& Tracks,
                            int SelectedTrackId)
{
    for (const auto& Ray : Rays) {
        sf::VertexArray Line(sf::Lines, 2);
        Line[0].position = WorldToScreen(Ray.Start);
        Line[1].position = WorldToScreen(Ray.End);
        sf::Color RayColor = Ray.IsBounce ? sf::Color(0, 255, 255, 100) : sf::Color(0, 255, 0, 15);
        Line[0].color = RayColor; Line[1].color = RayColor;
        RenderTarget.draw(Line);
    }

    for (const auto& Obj : Objects) {
        if (auto Ground = std::dynamic_pointer_cast<Obstacle>(Obj)) {
            sf::RectangleShape GroundRect(sf::Vector2f(RenderTarget.getSize().x, 500.0f));
            GroundRect.setFillColor(sf::Color(50, 80, 50));
            GroundRect.setPosition(0, WorldToScreen(Ground->GetPosition()).y);
            RenderTarget.draw(GroundRect);
        }
        else if (auto Tgt = std::dynamic_pointer_cast<Target>(Obj)) {
            sf::CircleShape TargetShape(4.0f);
            TargetShape.setFillColor(sf::Color::Red);
            TargetShape.setOrigin(4.0f, 4.0f);
            TargetShape.setPosition(WorldToScreen(Tgt->GetPosition()));
            RenderTarget.draw(TargetShape);
        }
    }

    for (const auto& Rdr : Radars) {
        sf::CircleShape RadarShape(8.0f);
        RadarShape.setFillColor(sf::Color(0, 255, 100));
        RadarShape.setOrigin(8.0f, 8.0f);
        RadarShape.setPosition(WorldToScreen(Rdr->GetPosition()));
        RenderTarget.draw(RadarShape);
    }

    // ИЗМЕНЕНО: Отрисовка объединенных треков и выделение цели
    for (const auto& Track : Tracks) {
        sf::RectangleShape TrackerMarker;

        if (Track.Id == SelectedTrackId) {
            // Цель захвачена для ракеты (Красный прицел)
            TrackerMarker.setSize(sf::Vector2f(26.0f, 26.0f));
            TrackerMarker.setOutlineColor(sf::Color::Red);
            TrackerMarker.setOrigin(13.0f, 13.0f);
        } else {
            // Обычная цель в сети (Желтая)
            TrackerMarker.setSize(sf::Vector2f(16.0f, 16.0f));
            TrackerMarker.setOutlineColor(sf::Color::Yellow);
            TrackerMarker.setOrigin(8.0f, 8.0f);
        }

        TrackerMarker.setFillColor(sf::Color::Transparent);
        TrackerMarker.setOutlineThickness(2.0f);
        TrackerMarker.setPosition(WorldToScreen(Track.Position));
        RenderTarget.draw(TrackerMarker);
    }
}