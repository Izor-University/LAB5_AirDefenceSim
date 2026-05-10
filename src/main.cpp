#include <iostream>
#include <memory>
#include <sstream>
#include <iomanip> // Для форматирования вывода (std::setprecision)
#include <SFML/System/Clock.hpp>

#include "../include/core/Vector3D.hpp"
#include "../include/entities/Target.hpp"
#include "../include/entities/Obstacle.hpp"
#include "../include/entities/Radar.hpp"
#include "../include/systems/SimulationWorld.hpp"
#include "../include/ui/SfmlVisualizer.hpp"

int main() {
    // Создаем мир с глубиной трассировки 2 (прямой луч + 1 рикошет от земли)
    SimulationWorld World(2);

    // ==========================================
    // ИНИЦИАЛИЗАЦИЯ СЕТИ РАДАРОВ (Тема 5)
    // ==========================================
    auto RadarEast = std::make_shared<Radar>("Radar_East", Vector3D(8000.0, 0.0, 0.0), 3e9);
    auto RadarWest = std::make_shared<Radar>("Radar_West", Vector3D(-8000.0, 0.0, 0.0), 3e9);

    RadarEast->SetRotationSpeed(2.5);
    RadarWest->SetRotationSpeed(2.0);

    // RadarEast стоит справа, сканирует запад (от 110 до 250 градусов)
    RadarEast->SetSector(110.0, 250.0);
    // RadarWest стоит слева, сканирует восток (от -70 до 70 градусов)
    RadarWest->SetSector(-70.0, 70.0);

    World.AddRadar(RadarEast);
    World.AddRadar(RadarWest);

    // ==========================================
    // ИНИЦИАЛИЗАЦИЯ ПРЕПЯТСТВИЙ (Тема 4.1)
    // ==========================================
    // Плоская земля на высоте Y = -100 м
    auto GroundPlane = std::make_shared<Obstacle>(
        "Ground", Vector3D(0.0, -100.0, 0.0), Vector3D(0.0, 1.0, 0.0)
    );
    World.AddObject(GroundPlane);

    // Хранилища для графического движка
    std::vector<std::shared_ptr<IPhysicalObject>> AllObjects = { GroundPlane };
    std::vector<std::shared_ptr<Radar>> AllRadars = { RadarEast, RadarWest };

    // ==========================================
    // НАСТРОЙКА ВИЗУАЛИЗАТОРА
    // ==========================================
    // Окно 1400x800, масштаб 1 пикс = 30 метров
    SfmlVisualizer UI(1400, 800, 1.0f / 30.0f);
    sf::Clock Clock;

    int TargetCounter = 1;
    double SimSpeed = 1.0;
    int SpawnMode = 1; // 1 - Ракета, 2 - Истребитель

    // ==========================================
    // ГЛАВНЫЙ ЦИКЛ (REAL-TIME)
    // ==========================================
    while (UI.IsOpen()) {

        // 1. Обработка ввода (Мышь и Клавиатура)
        auto ClickPos = UI.HandleEvents();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) SpawnMode = 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) SpawnMode = 2;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) SimSpeed += 0.01;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) SimSpeed = std::max(0.1, SimSpeed - 0.01);

        // Спавн новой цели по клику мыши
        if (ClickPos.has_value()) {
            double SpeedX = (SpawnMode == 1) ? -400.0 : 250.0; // Ракета быстрая и летит влево, самолет медленный и вправо
            double Rcs = (SpawnMode == 1) ? 5.0 : 25.0;        // ЭПР (эффективная площадь рассеяния)

            auto NewTarget = std::make_shared<Target>(
                "Target_" + std::to_string(TargetCounter++),
                ClickPos.value(),
                Vector3D(SpeedX, 0.0, 0.0),
                150.0, // Физический хитбокс
                Rcs
            );
            World.AddObject(NewTarget);
            AllObjects.push_back(NewTarget);
        }

        // 2. Расчет времени с учетом множителя скорости
        double DeltaTime = Clock.restart().asSeconds() * SimSpeed;

        // 3. Обновляем кинематику целей и вращение радаров
        World.UpdatePhysics(DeltaTime);
        for(auto& Rdr : AllRadars) {
            Rdr->UpdatePhysics(DeltaTime);
        }

        // 4. Сканирование и Data Fusion (Радары пускают лучи и сливают данные в сеть)
        World.ProcessRadarNetwork({});

        // 5. Формируем текст для панели управления (HUD)
        std::ostringstream StatsDisplay;
        StatsDisplay << std::fixed << std::setprecision(2); // Округляем числа до 2 знаков

        StatsDisplay << "=== AIR DEFENSE NETWORK ===\n\n";

        // Статусы радаров (Секторный поиск или Сопровождение цели)
        StatsDisplay << "[Radar West] Mode: "
                     << (RadarWest->GetMode() == RadarMode::TRACKING ? "TRACKING [!]" : "Scanning") << "\n";
        StatsDisplay << "[Radar East] Mode: "
                     << (RadarEast->GetMode() == RadarMode::TRACKING ? "TRACKING [!]" : "Scanning") << "\n\n";

        StatsDisplay << "Global Tracks (Found) : " << World.GetTracks().size() << "\n";
        StatsDisplay << "Real Flying Objects   : " << AllObjects.size() - 1 << "\n";
        StatsDisplay << "Simulation Speed      : " << SimSpeed << "x\n\n";

        StatsDisplay << "--- CONTROL PANEL ---\n";
        StatsDisplay << "Left Click : Spawn Target\n";
        StatsDisplay << "[1] Mode : Fast Missile (RCS 5)\n";
        StatsDisplay << "[2] Mode : Fighter Jet (RCS 25)\n";
        StatsDisplay << "[UP/DOWN]: Change Sim Speed\n\n";

        StatsDisplay << "Current Spawn Mode: " << (SpawnMode == 1 ? "MISSILE" : "FIGHTER") << "\n\n";

        StatsDisplay << "[Legend]\n";
        StatsDisplay << "Red Dot    : True Target Position\n";
        StatsDisplay << "Yellow [ ] : Network Radar Track\n";
        StatsDisplay << "Cyan Line  : Multipath Ground Bounce\n";

        // 6. Рендеринг кадра
        UI.Render(AllObjects, AllRadars, World.GetDebugRays(), World.GetTracks(), StatsDisplay.str());
    }

    return 0;
}