#include <iostream>
#include <memory>
#include <string>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "imgui.h"
#include "imgui-sfml.h"

#include "../include/core/Vector3D.hpp"
#include "../include/entities/Target.hpp"
#include "../include/entities/Obstacle.hpp"
#include "../include/entities/Radar.hpp"
#include "../include/systems/SimulationWorld.hpp"
#include "../include/ui/SfmlVisualizer.hpp"

int main() {
    SimulationWorld World(2);

    auto RadarEast = std::make_shared<Radar>("Radar_East", Vector3D(8000.0, 0.0, 0.0), 3e9);
    auto RadarWest = std::make_shared<Radar>("Radar_West", Vector3D(-8000.0, 0.0, 0.0), 3e9);

    RadarEast->SetSector(45.0, 135.0);
    RadarWest->SetSector(45.0, 135.0);

    World.AddRadar(RadarEast);
    World.AddRadar(RadarWest);

    auto GroundPlane = std::make_shared<Obstacle>("Ground", Vector3D(0.0, -100.0, 0.0), Vector3D(0.0, 1.0, 0.0));
    World.AddObject(GroundPlane);

    std::vector<std::shared_ptr<IPhysicalObject>> AllObjects = { GroundPlane };
    std::vector<std::shared_ptr<Radar>> AllRadars = { RadarEast, RadarWest };

    sf::RenderWindow Window(sf::VideoMode(1400, 800), "Air Defense Simulator + ImGui");
    Window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(Window)) { return -1; }

    SfmlVisualizer UI(Window, 1.0f / 30.0f);
    sf::Clock DeltaClock;
    int TargetCounter = 1;

    float SimSpeed = 1.0f;
    float SpawnSpeedX = -400.0f;
    float SpawnSpeedY = 0.0f;
    float SpawnRcs = 5.0f;
    float WestSector[2] = {45.0f, 135.0f};
    float EastSector[2] = {45.0f, 135.0f};

    // Лямбда-функция для отрисовки индивидуального дисплея радара
    auto DrawRadarPPI =[](std::shared_ptr<Radar> Rdr, const char* WindowTitle) {
        ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);
        ImGui::Begin(WindowTitle);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();

        draw_list->AddRectFilled(canvas_p0, ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y), IM_COL32(0, 15, 0, 255));

        ImVec2 center = ImVec2(canvas_p0.x + canvas_size.x / 2.0f, canvas_p0.y + canvas_size.y / 2.0f);
        float radius = (std::min(canvas_size.x, canvas_size.y) / 2.0f) * 0.9f;

        // Кольца дальности
        draw_list->AddCircle(center, radius, IM_COL32(0, 100, 0, 255), 64);
        draw_list->AddCircle(center, radius * 0.66f, IM_COL32(0, 100, 0, 255), 64);
        draw_list->AddCircle(center, radius * 0.33f, IM_COL32(0, 100, 0, 255), 64);
        draw_list->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(0, 100, 0, 255));
        draw_list->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(0, 100, 0, 255));

        // Линия сканирования (луч)
        double current_azimuth = Rdr->GetAzimuth();
        ImVec2 sweep_end(center.x + std::cos(current_azimuth) * radius, center.y - std::sin(current_azimuth) * radius);

        // Цвет луча: Зеленый при поиске, Красный при захвате!
        ImU32 RayColor = (Rdr->GetMode() == RadarMode::TRACKING) ? IM_COL32(255, 50, 50, 255) : IM_COL32(0, 255, 0, 255);
        draw_list->AddLine(center, sweep_end, RayColor, 2.0f);

        // Отрисовка ИНДИВИДУАЛЬНЫХ целей этого радара
        double MaxRadarRange = Rdr->GetMaxRange();
        for (const auto& Track : Rdr->GetLastTracks()) {
            double dx = Track.Position.X - Rdr->GetPosition().X;
            double dy = Track.Position.Y - Rdr->GetPosition().Y;
            double dist = std::sqrt(dx*dx + dy*dy);

            if (dist <= MaxRadarRange) {
                float norm_dist = dist / MaxRadarRange;
                float px = center.x + (dx / dist) * norm_dist * radius;
                float py = center.y - (dy / dist) * norm_dist * radius;

                draw_list->AddCircleFilled(ImVec2(px, py), 5.0f, IM_COL32(255, 255, 0, 255));

                char speed_text[32];
                snprintf(speed_text, sizeof(speed_text), "V: %.0f", std::abs(Track.RadialVelocity));
                draw_list->AddText(ImVec2(px + 8, py - 8), IM_COL32(255, 255, 255, 255), speed_text);
            }
        }
        ImGui::End();
    };

    while (Window.isOpen()) {
        sf::Event Event;
        while (Window.pollEvent(Event)) {
            ImGui::SFML::ProcessEvent(Window, Event);
            if (Event.type == sf::Event::Closed) Window.close();

            if (Event.type == sf::Event::MouseButtonPressed && Event.mouseButton.button == sf::Mouse::Left) {
                if (!ImGui::GetIO().WantCaptureMouse) {
                    Vector3D ClickWorldPos = UI.ScreenToWorld(sf::Vector2f(Event.mouseButton.x, Event.mouseButton.y));
                    auto NewTarget = std::make_shared<Target>(
                        "Target_" + std::to_string(TargetCounter++),
                        ClickWorldPos, Vector3D(SpawnSpeedX, SpawnSpeedY, 0.0), 150.0, SpawnRcs
                    );
                    World.AddObject(NewTarget);
                    AllObjects.push_back(NewTarget);
                }
            }
        }

        sf::Time dt = DeltaClock.restart();
        ImGui::SFML::Update(Window, dt);

        // --- ГЛАВНАЯ ПАНЕЛЬ ---
        ImGui::Begin("Command & Control Center");
        ImGui::Text("Global Settings");
        ImGui::SliderFloat("Simulation Speed", &SimSpeed, 0.0f, 5.0f);

        if (ImGui::Button("Clear All Targets", ImVec2(-1, 30))) {
            World.ClearTargets();
            auto it = AllObjects.begin();
            while (it != AllObjects.end()) {
                if ((*it)->GetName().find("Target") != std::string::npos) it = AllObjects.erase(it);
                else ++it;
            }
        }

        ImGui::Separator();
        ImGui::Text("Target Spawn Parameters");
        ImGui::SliderFloat("Velocity X (m/s)", &SpawnSpeedX, -1000.0f, 1000.0f);
        ImGui::SliderFloat("Velocity Y (m/s)", &SpawnSpeedY, -1000.0f, 1000.0f);
        ImGui::SliderFloat("RCS (m^2)", &SpawnRcs, 0.1f, 100.0f);

        ImGui::Separator();
        ImGui::Text("Radar Sectors (Degrees)");
        if (ImGui::SliderFloat2("West Radar", WestSector, 0.0f, 180.0f)) RadarWest->SetSector(WestSector[0], WestSector[1]);
        if (ImGui::SliderFloat2("East Radar", EastSector, 0.0f, 180.0f)) RadarEast->SetSector(EastSector[0], EastSector[1]);
        ImGui::End();

        // --- ДВА ТАКТИЧЕСКИХ ДИСПЛЕЯ ---
        DrawRadarPPI(RadarWest, "Tactical Display [WEST]");
        DrawRadarPPI(RadarEast, "Tactical Display [EAST]");

        // --- ФИЗИКА И РЕНДЕР ---
        double PhysicsDt = dt.asSeconds() * SimSpeed;
        World.UpdatePhysics(PhysicsDt);
        for(auto& Rdr : AllRadars) Rdr->UpdatePhysics(PhysicsDt);
        World.ProcessRadarNetwork({});

        Window.clear(sf::Color(15, 20, 30));
        UI.Render(AllObjects, AllRadars, World.GetDebugRays(), World.GetTracks());
        ImGui::SFML::Render(Window);
        Window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}