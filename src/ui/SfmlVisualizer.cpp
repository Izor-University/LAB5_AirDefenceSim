#include "../../include/ui/SfmlVisualizer.hpp"
#include <iostream>

SfmlVisualizer::SfmlVisualizer(int Width, int Height, float Scale)
    : Window(sf::VideoMode(Width, Height), "Air Defense Simulator"),
      Scale(Scale),
      CameraOffset(Width / 2.0f, Height / 2.0f)
{
    Window.setFramerateLimit(60);

    // Загрузка шрифта
    if (!MainFont.loadFromFile("arial.ttf")) {
        std::cerr << "ВНИМАНИЕ: Не найден шрифт arial.ttf в корне проекта! Текст не будет отображаться.\n";
    } else {
        InfoText.setFont(MainFont);
        InfoText.setCharacterSize(18);
        InfoText.setFillColor(sf::Color::White);
        InfoText.setPosition(10.0f, 10.0f);
    }
}

sf::Vector2f SfmlVisualizer::WorldToScreen(const Vector3D& WorldPos) const {
    return sf::Vector2f(CameraOffset.x + (WorldPos.X * Scale), CameraOffset.y - (WorldPos.Y * Scale));
}

Vector3D SfmlVisualizer::ScreenToWorld(const sf::Vector2f& ScreenPos) const {
    double X = (ScreenPos.x - CameraOffset.x) / Scale;
    double Y = (CameraOffset.y - ScreenPos.y) / Scale;
    return Vector3D(X, Y, 0.0);
}

bool SfmlVisualizer::IsOpen() const { return Window.isOpen(); }

std::optional<Vector3D> SfmlVisualizer::HandleEvents() {
    sf::Event Event;
    std::optional<Vector3D> ClickPosition = std::nullopt;

    while (Window.pollEvent(Event)) {
        if (Event.type == sf::Event::Closed) {
            Window.close();
        }
        // Обработка клика мышкой (добавление цели)
        if (Event.type == sf::Event::MouseButtonPressed) {
            if (Event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f MousePos(Event.mouseButton.x, Event.mouseButton.y);
                ClickPosition = ScreenToWorld(MousePos);
            }
        }
    }
    return ClickPosition;
}

void SfmlVisualizer::Render(const std::vector<std::shared_ptr<IPhysicalObject>>& Objects,
                            const std::vector<std::shared_ptr<Radar>>& Radars,
                            const std::vector<RaySegment>& Rays,
                            const std::vector<Vector3D>& Tracks, // <--- Новый параметр!
                            const std::string& RadarStats)
{
    Window.clear(sf::Color(15, 20, 30));

    // 1. Отрисовка лучей сканирования
    for (const auto& Ray : Rays) {
        sf::VertexArray Line(sf::Lines, 2);
        Line[0].position = WorldToScreen(Ray.Start);
        Line[1].position = WorldToScreen(Ray.End);
        sf::Color RayColor = Ray.IsBounce ? sf::Color(0, 255, 255, 100) : sf::Color(0, 255, 0, 15); // Зеленая подсветка радара
        Line[0].color = RayColor; Line[1].color = RayColor;
        Window.draw(Line);
    }

    // 2. Отрисовка Земли и Целей (Ground Truth - то, что есть на самом деле)
    for (const auto& Obj : Objects) {
        if (auto Ground = std::dynamic_pointer_cast<Obstacle>(Obj)) {
            sf::RectangleShape GroundRect(sf::Vector2f(Window.getSize().x, 500.0f));
            GroundRect.setFillColor(sf::Color(50, 80, 50));
            GroundRect.setPosition(0, WorldToScreen(Ground->GetPosition()).y);
            Window.draw(GroundRect);
        }
        else if (auto Tgt = std::dynamic_pointer_cast<Target>(Obj)) {
            sf::CircleShape TargetShape(4.0f);
            TargetShape.setFillColor(sf::Color::Red);
            TargetShape.setOrigin(4.0f, 4.0f);
            TargetShape.setPosition(WorldToScreen(Tgt->GetPosition()));
            Window.draw(TargetShape);
        }
    }

    // 3. Отрисовка Радаров
    for (const auto& Rdr : Radars) {
        sf::CircleShape RadarShape(8.0f);
        RadarShape.setFillColor(sf::Color(0, 255, 100));
        RadarShape.setOrigin(8.0f, 8.0f);
        RadarShape.setPosition(WorldToScreen(Rdr->GetPosition()));
        Window.draw(RadarShape);
    }

    // 4. Отрисовка Сетевых Треков (То, что "увидела" ПВО) - Тема 5
    for (const auto& TrackPos : Tracks) {
        sf::RectangleShape TrackerMarker(sf::Vector2f(16.0f, 16.0f));
        TrackerMarker.setFillColor(sf::Color::Transparent);
        TrackerMarker.setOutlineThickness(2.0f);
        TrackerMarker.setOutlineColor(sf::Color::Yellow);
        TrackerMarker.setOrigin(8.0f, 8.0f);
        TrackerMarker.setPosition(WorldToScreen(TrackPos));
        Window.draw(TrackerMarker);
    }

    // 5. Боковая Панель Управления (UI)
    sf::RectangleShape UIBackground(sf::Vector2f(300.0f, Window.getSize().y));
    UIBackground.setFillColor(sf::Color(20, 25, 35, 230));
    Window.draw(UIBackground);

    InfoText.setString(RadarStats);
    Window.draw(InfoText);

    Window.display();
}