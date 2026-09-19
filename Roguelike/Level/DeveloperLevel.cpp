#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "GameResources.h"
#include "SpeechDirector.h"
#include "LevelBuilder.h"
#include "ActAssembler.h"
#include "LevelLoader.h"
#include "LevelProgression.h"
#include "Player.h"
#include "Music.h"
#include "Crosshair.h"
#include "Particles.h"
#include "BossBrainComponent.h"
#include "CameraDirectorComponent.h"
#include "CarArrival.h"
#include "ChaseComponent.h"
#include "Freeze.h"
#include "ProjectileComponent.h"
#include "TrailComponent.h"
#include "SwitchComponent.h"
#include "CutscenePlayerComponent.h"
#include "Item.h"
#include "ItemDropComponent.h"
#include "ItemPickupComponent.h"
#include "PlayerHudBinderComponent.h"
#include "EscapeCarComponent.h"
#include "PursuitComponent.h"
#include "WaveDirectorComponent.h"
#include "Fx.h"
#include "LevelExitComponent.h"
#include "UiRoot.h"
#include <Engine.h>
#include <GameWorld.h>
#include <ParticleSystem.h>
#include <UiManager.h>
#include <FrameClock.h>
#include <InputSystem.h>
#include <RectangleRendererComponent.h>
#include <RenderSystem.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <MusicComponent.h>
#include "HealthComponent.h"
#include <LoggerRegistry.h>
#include <string>

using namespace XYZEngine;

namespace RoguelikeGame
{
    void DeveloperLevel::Start()
    {
        LOG_INFO("Developer level is starting");

        state = RunState::Playing;
        gameOverDelay.Stop();

        // Начинаем с первой сюжетной локации, а не с первой строки реестра.
        int firstLevel = GameResources::GetLevels().FirstIndex(LevelMode::Campaign);
        currentLevelIndex = firstLevel < 0 ? 0 : firstLevel;
        pendingLevelIndex = -1;

        LoadLevel(currentLevelIndex);

        particles = CreateParticles();

        {
            try
            {
                player = CreatePlayer(level.GetStartPosition());
                camera = CreateCamera(player);

                auto drop = player->GetComponent<ItemDropComponent>();
                if (drop != nullptr)
                {
                    // Объект без уровня переживёт очистку и всплывёт на следующей локации сиротой.
                    drop->SetSpawner([this](const ItemDefinition& item, int count, int charge,
                        const XYZEngine::Vector2Df& place)
                    {
                        XYZEngine::GameObject* dropped = CreateItem(item, place);
                        if (dropped == nullptr)
                        {
                            return false;
                        }

                        dropped->GetComponent<ItemPickupComponent>()->SetStack(count, charge);

                        return level.Add(dropped);
                    });
                }

                auto health = player->GetComponent<HealthComponent>();
                if (health != nullptr)
                {
                    health->SubscribeDeath([this](const DeathInfo& death)
                    {
                        state = RunState::PlayerDied;
                        gameOverDelay.Start(GAME_OVER_DELAY);
                        SilenceCar();
                    });
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERROR(std::string("Player is not created: ") + exception.what());
            }
        }

        SubscribeExit();
        SubscribeBoss();
        SubscribeWaves();
        SubscribePursuit();
        SubscribeEscape();
        SubscribeLevers();

        music = CreateMusic(MAIN_THEME_MUSIC, MUSIC_VOLUME);

        try
        {
            crosshair = CreateCrosshair();
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Crosshair is not created: ") + exception.what());
        }

        hudScreen = std::make_unique<HudScreen>();
        inventoryScreen = std::make_unique<InventoryScreen>();
        messageScreen = std::make_unique<MessageScreen>();
        fadeScreen = std::make_unique<FadeScreen>();

        subtitleScreen = std::make_unique<SubtitleScreen>();
        subtitleScreen->SetQueue(&subtitles);
        SpeechDirector::Current().SetCatalog(&GameResources::GetSpeech());
        SpeechDirector::Current().SetQueue(&subtitles);

        uiRoot = CreateUiRoot(*hudScreen, *subtitleScreen, *inventoryScreen, *messageScreen, *fadeScreen);

        fadeScreen->Blackout();
        fadeScreen->FadeIn(LEVEL_FADE_IN_TIME);

        ShowLevelTitle();
    }

    bool DeveloperLevel::LoadLevel(int levelIndex)
    {
        const LevelEntry* entry = GameResources::GetLevels().GetAt(levelIndex);
        std::string levelFile = entry != nullptr ? entry->filePath : TEST_LEVEL_FILE;

        try
        {
            bool isAct = entry != nullptr && entry->isAct;
            LevelData levelData = isAct ? LoadAct(levelFile) : LevelLoader::Load(levelFile);
            level = LevelBuilder::Build(levelData, GameResources::GetItems(), GameResources::GetProps());
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Level is not loaded: ") + exception.what());
            LOG_WARN("Game continues with an empty level");
            return false;
        }

        currentLevelIndex = levelIndex;
        return true;
    }

    void DeveloperLevel::SubscribeExit()
    {
        XYZEngine::GameObject* exitObject = level.GetExit();
        if (exitObject == nullptr)
        {
            return;
        }

        auto exitComponent = exitObject->GetComponent<LevelExitComponent>();
        if (exitComponent == nullptr)
        {
            return;
        }

        exitComponent->SubscribeEntered([this]() { RequestNextLevel(); });
        exitComponent->SubscribeBlocked([this]()
        {
            if (hudScreen != nullptr)
            {
                hudScreen->ShowNotice(level.GetWaveDirector() != nullptr ? WAVE_GATE_NOTICE : BOSS_GATE_NOTICE);
            }
        });
    }

    void DeveloperLevel::SubscribeBoss()
    {
        XYZEngine::GameObject* bossObject = level.GetBoss();
        if (bossObject == nullptr)
        {
            return;
        }

        auto bossHealth = bossObject->GetComponent<HealthComponent>();
        if (bossHealth != nullptr)
        {
            bossHealth->SubscribeDeath([this](const DeathInfo& death) { OnBossDefeated(); });
        }

        auto brain = bossObject->GetComponent<BossBrainComponent>();
        if (brain != nullptr)
        {
            brain->SubscribeMinionSpawned([this](XYZEngine::GameObject* minion) { level.Add(minion); });
        }
    }

    void DeveloperLevel::SubscribeWaves()
    {
        XYZEngine::GameObject* directorObject = level.GetWaveDirector();
        if (directorObject == nullptr)
        {
            return;
        }

        auto director = directorObject->GetComponent<WaveDirectorComponent>();
        if (director == nullptr)
        {
            return;
        }

        director->SetHero(player);

        // Врагов волны кладём в уровень, иначе они переживут смену локации.
        director->SubscribeEnemySpawned([this](XYZEngine::GameObject* enemy) { level.Add(enemy); });

        director->SubscribeWaveStarted([this](int current, int total)
        {
            if (hudScreen != nullptr)
            {
                hudScreen->ShowNotice(std::string(WAVE_STARTED_NOTICE) + std::to_string(current)
                    + WAVE_OF_NOTICE + std::to_string(total));
            }
        });

        director->SubscribeWaveCleared([this](int current, int total)
        {
            if (hudScreen != nullptr && current < total)
            {
                hudScreen->ShowNotice(std::string(WAVE_STARTED_NOTICE) + std::to_string(current) + WAVE_CLEARED_NOTICE);
            }
        });

        director->SubscribeCleared([this]() { OnWavesCleared(); });
    }


    /**
    *	Погоня бежит за игроком от входа до машины: этот отрезок и есть её маршрут.
    *	Гейта у неё нет - машина открыта сразу, преградой служит сама дистанция.
    */
    void DeveloperLevel::SubscribePursuit()
    {
        XYZEngine::GameObject* pursuitObject = level.GetPursuit();
        if (pursuitObject == nullptr)
        {
            return;
        }

        auto pursuit = pursuitObject->GetComponent<PursuitComponent>();
        if (pursuit == nullptr)
        {
            return;
        }

        pursuit->SetHero(player);

        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        auto waiting = carObject != nullptr ? carObject->GetComponent<EscapeCarComponent>() : nullptr;

        // Финиш забега - место машины, а не сама машина: на время приезда она в стороне.
        XYZEngine::Vector2Df finish = waiting != nullptr ? waiting->GetParkPlace() : level.GetStartPosition();
        pursuit->SetRoute(level.GetStartPosition(), finish);

        // Преследователей кладём в уровень, иначе они переживут смену локации.
        pursuit->SubscribeEnemySpawned([this](XYZEngine::GameObject* enemy) { level.Add(enemy); });

        if (carObject != nullptr)
        {
            auto car = carObject->GetComponent<EscapeCarComponent>();
            if (car != nullptr)
            {
                car->SetReady(true);
            }
        }
    }

    void DeveloperLevel::SubscribeEscape()
    {
        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject == nullptr)
        {
            return;
        }

        auto car = carObject->GetComponent<EscapeCarComponent>();
        if (car == nullptr)
        {
            return;
        }

        car->SubscribeBoarded([this]() { PlayEscape(); });

        car->SetHero(player);
        car->SubscribeCalled([this]() { PlayArrival(); });
        SendCarAway(carObject, car);
    }

    // До приезда машины на карте нет: ни спрайта, ни подсказки.
    void DeveloperLevel::SendCarAway(XYZEngine::GameObject* carObject, EscapeCarComponent* car)
    {
        car->SetArrived(false);

        CarPose start = ArrivalPose(car->GetParkPlace(), 0.f);
        carObject->GetTransform()->SetWorldPosition(start.place);
        carObject->GetTransform()->SetWorldRotation(start.angle);

        ShowCar(carObject, false);
    }

    void DeveloperLevel::ShowCar(XYZEngine::GameObject* carObject, bool isShown)
    {
        auto sprite = carObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        if (sprite != nullptr)
        {
            sprite->SetVisible(isShown);
        }
    }

    // Дверь - второй кадр, он вдвое выше кузова, поэтому меняется и размер.
    void DeveloperLevel::OpenCarDoor(XYZEngine::GameObject* carObject, bool isOpen)
    {
        auto sprite = carObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        if (sprite == nullptr)
        {
            return;
        }

        const sf::Texture* body = XYZEngine::ResourceSystem::Instance()->GetTextureShared(
            isOpen ? ESCAPE_CAR_OPEN_TEXTURE : ESCAPE_CAR_TEXTURE);
        if (body == nullptr)
        {
            return;
        }

        sprite->SetTexture(*body);
        sprite->SetPixelSize(static_cast<int>(ESCAPE_CAR_WIDTH),
            static_cast<int>(isOpen ? ESCAPE_CAR_OPEN_HEIGHT : ESCAPE_CAR_HEIGHT));
    }

    /**
    *	Приезд: камера уходит вперёд на пустое место, туда влетает машина.
    *
    *	Камера смотрит на точку, а не на машину: иначе кадр уехал бы за горизонт
    *	и приезжать было бы некуда.
    */
    void DeveloperLevel::PlayArrival()
    {
        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject == nullptr)
        {
            return;
        }

        auto car = carObject->GetComponent<EscapeCarComponent>();
        if (car == nullptr)
        {
            return;
        }

        CutsceneBeat hold;
        hold.command = CutsceneCommand::TakeControl;

        CutsceneBeat look;
        look.command = CutsceneCommand::LookAtPoint;
        look.point = car->GetParkPlace();
        look.travel = ARRIVAL_LOOK_TIME;
        look.seconds = ARRIVAL_LOOK_TIME;

        CutsceneBeat drive;
        drive.action = ARRIVAL_BEAT_DRIVE;
        drive.seconds = ARRIVAL_DRIVE_TIME;

        CutsceneBeat door;
        door.action = ARRIVAL_BEAT_DOOR;
        door.seconds = ARRIVAL_DOOR_TIME;

        CutsceneBeat back;
        back.command = CutsceneCommand::LookAtHero;
        back.travel = ARRIVAL_LOOK_TIME;
        back.seconds = ARRIVAL_LOOK_TIME;

        CutscenePlayerComponent* scene = StartCutscene({hold, look, drive, door, back});
        if (scene == nullptr)
        {
            return;
        }

        arrivalTime = 0.f;
        smokeTime = 0.f;
        wasSkidding = false;

        scene->SetHandler(ARRIVAL_BEAT_DRIVE, [this, carObject](float deltaTime)
        {
            DriveArrival(carObject, deltaTime);
        });

        scene->SubscribeBeatStarted([this, carObject, car](const std::string& beat)
        {
            if (beat == ARRIVAL_BEAT_DRIVE)
            {
                ShowCar(carObject, true);
                car->StartEngine();
            }

            if (beat == ARRIVAL_BEAT_DOOR)
            {
                OpenCarDoor(carObject, true);
                car->SetArrived(true);
                car->StopEngine();
            }
        });
    }

    /**
    *	Дым и следы идут, пока машину тащит юзом, и тем гуще, чем сильнее занос.
    *
    *	След ложится по направлению движения, а не по корпусу: в заносе машина развёрнута
    *	поперёк, а тащит её вперёд - в этом вся картинка.
    */
    void DeveloperLevel::TrailSkid(EscapeCarComponent* car, const CarPose& pose, const XYZEngine::Vector2Df& step, float deltaTime)
    {
        if (pose.skid <= 0.f)
        {
            return;
        }

        if (!wasSkidding)
        {
            wasSkidding = true;
            car->Screech();
        }

        smokeTime += deltaTime * pose.skid;
        while (smokeTime >= ARRIVAL_SMOKE_STEP)
        {
            smokeTime -= ARRIVAL_SMOKE_STEP;

            for (bool isLeft : {true, false})
            {
                Fx::SpawnTireSmoke(WheelPlace(pose, isLeft), ARRIVAL_SMOKE_SCALE);
            }
        }

        // Почти вставшая машина резину не жжёт, а её направление - уже шум.
        if (deltaTime <= 0.f || step.GetLength() / deltaTime < ARRIVAL_MARK_MIN_SPEED)
        {
            return;
        }

        float alpha = TIRE_MARK_MIN_ALPHA + (1.f - TIRE_MARK_MIN_ALPHA) * std::min(1.f, pose.skid);

        for (int wheel = 0; wheel < 2; wheel++)
        {
            TrailComponent* trail = SkidTrail(wheel);
            if (trail != nullptr)
            {
                trail->Add(WheelPlace(pose, wheel == 0), alpha);
            }
        }
    }

    /**
    *	Лента следа под одним колесом, создаётся при первом же заносе.
    *
    *	Имя то же, что у прежних штампов: смена локации выметает их по нему.
    */
    TrailComponent* DeveloperLevel::SkidTrail(int wheel)
    {
        if (wheel < 0 || wheel > 1)
        {
            return nullptr;
        }

        if (skidTrails[wheel] == nullptr)
        {
            auto gameObject = GameWorld::Instance()->CreateGameObject(TIRE_MARK_OBJECT_NAME);
            gameObject->SetTemporary(true);
            gameObject->SetRenderLayer(TIRE_MARK_RENDER_LAYER);

            auto trail = gameObject->AddComponent<TrailComponent>();
            trail->SetColour(TIRE_MARK_COLOR);
            trail->SetWidth(TIRE_MARK_WIDTH);
            trail->SetStep(ARRIVAL_MARK_STEP);

            skidTrails[wheel] = trail;
        }

        return skidTrails[wheel];
    }

    void DeveloperLevel::SilenceCar()
    {
        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject == nullptr)
        {
            return;
        }

        auto car = carObject->GetComponent<EscapeCarComponent>();
        if (car != nullptr)
        {
            car->StopEngine();
        }
    }

    void DeveloperLevel::DriveArrival(XYZEngine::GameObject* carObject, float deltaTime)
    {
        auto car = carObject->GetComponent<EscapeCarComponent>();
        if (car == nullptr)
        {
            return;
        }

        arrivalTime += deltaTime;

        CarPose pose = ArrivalPose(car->GetParkPlace(), arrivalTime / ARRIVAL_DRIVE_TIME);
        XYZEngine::Vector2Df step = pose.place - carObject->GetTransform()->GetWorldPosition();

        carObject->GetTransform()->SetWorldPosition(pose.place);
        carObject->GetTransform()->SetWorldRotation(pose.angle);

        TrailSkid(car, pose, step, deltaTime);
    }

    /**
    *	Рычаг открывает люк где-то в стороне, и без камеры игрок видит только
    *	то, что дёрнул рычаг. Сцена показывает результат: камера уезжает к люку,
    *	ждёт, пока он откроется, и возвращается.
    */
    void DeveloperLevel::SubscribeLevers()
    {
        for (SwitchComponent* lever : GameWorld::Instance()->FindComponents<SwitchComponent>())
        {
            std::string hatchName = std::string(HATCH_OBJECT_PREFIX) + lever->GetSwitchId();
            if (GameWorld::Instance()->FindGameObject(hatchName) == nullptr)
            {
                continue;
            }

            lever->SubscribePulled([this, hatchName]() { PlayHatchScene(hatchName); });
        }
    }

    void DeveloperLevel::PlayHatchScene(const std::string& hatchName)
    {
        CutsceneBeat away;
        away.action = HATCH_SCENE_BEAT;
        away.seconds = HATCH_SCENE_TRAVEL + HATCH_SCENE_HOLD;
        away.command = CutsceneCommand::LookAtTarget;
        away.target = hatchName;
        away.travel = HATCH_SCENE_TRAVEL;

        CutsceneBeat back;
        back.seconds = HATCH_SCENE_TRAVEL;
        back.command = CutsceneCommand::LookAtHero;
        back.travel = HATCH_SCENE_TRAVEL;

        CutsceneBeat hold;
        hold.command = CutsceneCommand::TakeControl;
        hold.seconds = 0.f;

        StartCutscene({hold, away, back});
    }

    /**
    *	Собирает сцену: общая обвязка у всех одна - камера, замок управления
    *	и поиск цели по имени. Сами шаги приносит тот, кто сцену заказывает.
    */
    CutscenePlayerComponent* DeveloperLevel::StartCutscene(std::vector<CutsceneBeat> beats)
    {
        if (cutscene != nullptr)
        {
            return nullptr;
        }

        // Сцена принадлежит только себе: она сама себя убирает по окончании,
        // а уровень держал бы на неё уже мёртвый указатель до следующей локации.
        cutscene = XYZEngine::GameWorld::Instance()->CreateGameObject(CUTSCENE_OBJECT_NAME);
        cutscene->SetTemporary(true);

        auto scene = cutscene->AddComponent<CutscenePlayerComponent>();
        scene->SetBeats(std::move(beats));
        scene->SetControlLock([this](bool isTaken) { SetControlTaken(isTaken); });
        scene->SetTargetFinder([](const std::string& name)
        {
            return XYZEngine::GameWorld::Instance()->FindGameObject(name);
        });

        if (camera != nullptr)
        {
            scene->SetCamera(camera->GetComponent<CameraDirectorComponent>());
        }

        scene->SubscribeFinished([this]()
        {
            // Сцена отыграла - объект больше не нужен, и место под следующую свободно.
            XYZEngine::GameWorld::Instance()->DestroyGameObject(cutscene);
            cutscene = nullptr;
        });

        scene->Play();

        return scene;
    }

    void DeveloperLevel::PlayEscape()
    {
        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject == nullptr || player == nullptr || cutscene != nullptr)
        {
            return;
        }

        cutscene = XYZEngine::GameWorld::Instance()->CreateGameObject(CUTSCENE_OBJECT_NAME);
        cutscene->SetTemporary(true);

        auto scene = cutscene->AddComponent<CutscenePlayerComponent>();
        scene->SetBeats({
            {ESCAPE_BEAT_BOARD, ESCAPE_BOARD_TIME},
            {ESCAPE_BEAT_DRIVE, ESCAPE_DRIVE_TIME},
            {ESCAPE_BEAT_LEAVE, ESCAPE_LEAVE_TIME},
        });

        // Игрок становится пассажиром: его выключают, а камера едет вместе с машиной.
        scene->SubscribeBeatStarted([this, carObject](const std::string& beat)
        {
            if (beat == ESCAPE_BEAT_BOARD)
            {
                TakeControl();
                OpenCarDoor(carObject, false);

                auto car = carObject->GetComponent<EscapeCarComponent>();
                if (car != nullptr)
                {
                    car->StartEngine();
                }

                // Герой внутри машины - не видно ни его, ни оружия у него в руках.
                // Оружие - отдельный дочерний объект, и гасится он только вместе с хозяином.
                if (player != nullptr)
                {
                    player->SetActive(false);
                }

                if (hudScreen != nullptr)
                {
                    hudScreen->ShowNotice(ESCAPE_NOTICE);
                }
            }

            // Длинный кадр может перешагнуть весь такт посадки - тогда доворот не успеет
            // отыграться ни разу, и машина уедет боком.
            if (beat == ESCAPE_BEAT_DRIVE)
            {
                carObject->GetTransform()->SetWorldRotation(ESCAPE_FACING_OUT);
            }

            if (beat == ESCAPE_BEAT_LEAVE && fadeScreen != nullptr)
            {
                fadeScreen->FadeOut(ESCAPE_LEAVE_TIME);
            }
        });

        boardTime = 0.f;
        escapeSpeed = 0.f;

        // Машина стоит поперёк, а ехать ей направо: доворачивает, пока закрывается дверь.
        scene->SetHandler(ESCAPE_BEAT_BOARD, [this, carObject](float deltaTime)
        {
            boardTime += deltaTime;
            carObject->GetTransform()->SetWorldRotation(BoardingAngle(boardTime / ESCAPE_BOARD_TIME));
            DriveEscape(carObject, deltaTime);
        });

        scene->SetHandler(ESCAPE_BEAT_DRIVE, [this, carObject](float deltaTime)
        {
            DriveEscape(carObject, deltaTime);
        });

        scene->SetHandler(ESCAPE_BEAT_LEAVE, [this, carObject](float deltaTime)
        {
            DriveEscape(carObject, deltaTime);
        });

        scene->SubscribeFinished([this]() { RequestNextLevel(); });
        scene->Play();
    }

    /**
    *	Сцена забирает мир себе: замирает и герой, и противники, и пули в воздухе.
    *
    *	Заморозка гасит только поведение и не трогает рисование: иначе на время
    *	сцены все пропадут с экрана - включая самого героя, к которому камера возвращается в конце.
    */
    void DeveloperLevel::SetControlTaken(bool isTaken)
    {
        if (!isTaken)
        {
            Thaw(takenParts);

            if (crosshair != nullptr)
            {
                crosshair->SetActive(true);
            }

            return;
        }

        if (!takenParts.empty())
        {
            return;
        }

        RoguelikeGame::Freeze(player, takenParts);

        // Компонент преследования есть на каждом враге, включая босса - по нему их и ищут.
        for (ChaseComponent* enemy : GameWorld::Instance()->FindComponents<ChaseComponent>())
        {
            RoguelikeGame::Freeze(enemy->GetGameObject(), takenParts);
        }

        // Пуля замирает вместе со своим сроком жизни: за время сцены ни одна не истечёт.
        for (ProjectileComponent* bullet : GameWorld::Instance()->FindComponents<ProjectileComponent>())
        {
            if (bullet->IsEnabled())
            {
                bullet->SetEnabled(false);
                takenParts.push_back({bullet->GetGameObject(), bullet});
            }
        }

        if (crosshair != nullptr)
        {
            crosshair->SetActive(false);
        }
    }
    void DeveloperLevel::TakeControl()
    {
        SetControlTaken(true);
    }

    void DeveloperLevel::DriveEscape(XYZEngine::GameObject* carObject, float deltaTime)
    {
        if (carObject == nullptr)
        {
            return;
        }

        escapeSpeed = std::min(ESCAPE_CAR_SPEED, escapeSpeed + ESCAPE_CAR_PICKUP * deltaTime);

        // Едет вдоль собственного носа: пока корпус доворачивается, машина уже
        // катится, и получается дуга, а не пируэт на месте.
        auto transform = carObject->GetTransform();
        transform->MoveBy(transform->GetForward() * (escapeSpeed * deltaTime));

        if (player != nullptr)
        {
            player->GetTransform()->SetWorldPosition(carObject->GetTransform()->GetWorldPosition());
        }
    }

    void DeveloperLevel::OnWavesCleared()
    {
        XYZEngine::GameObject* exitObject = level.GetExit();
        if (exitObject != nullptr)
        {
            auto exitComponent = exitObject->GetComponent<LevelExitComponent>();
            if (exitComponent != nullptr)
            {
                exitComponent->SetLocked(false);
            }

            auto renderer = exitObject->GetComponent<XYZEngine::RectangleRendererComponent>();
            if (renderer != nullptr)
            {
                renderer->SetColor(LEVEL_EXIT_COLOR);
            }
        }

        XYZEngine::GameObject* carObject = level.GetEscapeCar();
        if (carObject != nullptr)
        {
            auto car = carObject->GetComponent<EscapeCarComponent>();
            if (car != nullptr)
            {
                car->SetReady(true);
            }
        }

        if (hudScreen != nullptr)
        {
            hudScreen->ShowNotice(WAVES_DONE_NOTICE);
        }
    }

    void DeveloperLevel::OnBossDefeated()
    {
        XYZEngine::GameObject* exitObject = level.GetExit();
        if (exitObject != nullptr)
        {
            auto exitComponent = exitObject->GetComponent<LevelExitComponent>();
            if (exitComponent != nullptr)
            {
                exitComponent->SetLocked(false);
            }

            auto renderer = exitObject->GetComponent<XYZEngine::RectangleRendererComponent>();
            if (renderer != nullptr)
            {
                renderer->SetColor(LEVEL_EXIT_COLOR);
            }
        }

        XYZEngine::FrameClock::Instance()->SlowMotion(DEATH_TIME_SCALE, DEATH_SLOW_MOTION_TIME, DEATH_SLOW_MOTION_BLEND);
        Fx::ShakeCamera(CAMERA_SHAKE_HEAVY);

        if (hudScreen != nullptr)
        {
            hudScreen->ShowNotice(BOSS_DEFEATED_NOTICE);
        }

        LOG_INFO("Boss is defeated, the level exit is open");
    }

    void DeveloperLevel::RequestNextLevel()
    {
        const LevelCatalog& levels = GameResources::GetLevels();
        LevelStep step = ResolveNextLevel(levels, currentLevelIndex, level.GetInfo().nextLevelId);

        if (step.kind == LevelStepKind::Unknown)
        {
            LOG_ERROR("Unknown next level: " + level.GetInfo().nextLevelId);
            return;
        }

        if (step.kind == LevelStepKind::Finished)
        {
            LOG_INFO("No more levels, the run is complete");
            state = RunState::Victory;
            UpdateOverlay();
            return;
        }

        pendingLevelIndex = step.index;

        if (fadeScreen != nullptr)
        {
            fadeScreen->FadeOut(LEVEL_FADE_OUT_TIME);
        }
    }

    void DeveloperLevel::GoToPendingLevel()
    {
        int nextIndex = pendingLevelIndex;
        pendingLevelIndex = -1;

        const LevelEntry* entry = GameResources::GetLevels().GetAt(nextIndex);
        if (entry == nullptr)
        {
            return;
        }

        // Сцена кончилась сменой локации: размораживать надо до того, как умрут
        // замороженные. Иначе список останется с указателями на чужую память,
        // а игрок приедет на новую локацию с выключенными компонентами.
        SetControlTaken(false);

        level.Clear();

        // Список имён забывали пополнять - так огонь и переезжал на следующую локацию.
        // Теперь решение принимается там, где объект создают.
        GameWorld::Instance()->DestroyTemporary();

        // Недоигравшая сцена осталась на прежней локации вместе со своим объектом.
        cutscene = nullptr;

        XYZEngine::ParticleSystem::Instance()->Clear();
        GameWorld::Instance()->LateUpdate();

        if (!LoadLevel(nextIndex))
        {
            return;
        }

        if (player != nullptr)
        {
            // Герой уехал выключенным, сидя в машине - на новой локации он снова на ногах.
            player->SetActive(true);
            player->GetTransform()->SetWorldPosition(level.GetStartPosition());
        }

        SubscribeExit();
        SubscribeBoss();
        SubscribeWaves();
        SubscribePursuit();
        SubscribeEscape();
        SubscribeLevers();
        ShowLevelTitle();

        LOG_INFO("Level changed to " + entry->id + ", objects in world "
            + std::to_string(GameWorld::Instance()->GetObjectsCount()));
    }

    void DeveloperLevel::ShowLevelTitle()
    {
        if (hudScreen == nullptr)
        {
            return;
        }

        const LevelEntry* entry = GameResources::GetLevels().GetAt(currentLevelIndex);
        const std::string& title = level.GetInfo().title.empty() && entry != nullptr
            ? entry->title
            : level.GetInfo().title;

        if (!title.empty())
        {
            hudScreen->ShowNotice(title.c_str());
        }
    }

    void DeveloperLevel::Update(float deltaTime)
    {
        if (pendingLevelIndex >= 0 && (fadeScreen == nullptr || fadeScreen->IsCovered()))
        {
            GoToPendingLevel();

            if (fadeScreen != nullptr)
            {
                fadeScreen->FadeIn(LEVEL_FADE_IN_TIME);
            }
        }

        auto input = InputSystem::Instance();
        bool isPaused = Engine::Instance()->IsPaused();

        // Escape при открытой сумке закрывает её, а не ставит игру на паузу.
        if (MayPause(state) && !XYZEngine::UiManager::Instance()->IsInputCaptured()
            && input->WasActionPressed(XYZEngine::InputAction::Pause))
        {
            SetPaused(!isPaused);
            return;
        }
        if (MayPause(state) && !input->HasFocus() && !isPaused)
        {
            SetPaused(true);
            return;
        }
        if (isPaused)
        {
            return;
        }

        UpdateWavePanel();
        UpdateChasePanel();

        if (state == RunState::PlayerDied)
        {
            gameOverDelay.Tick(deltaTime);
            if (gameOverDelay.IsReady())
            {
                ShowGameOver();
            }
        }
        else if ((state == RunState::GameOver || state == RunState::Victory) && input->WasKeyPressed(RESTART_KEY))
        {
            Restart();
        }
        else if (input->WasKeyPressed(DEBUG_HEAL_KEY))
        {
            auto health = GameWorld::Instance()->FindComponent<HealthComponent>(PLAYER_OBJECT_NAME);
            if (health != nullptr)
            {
                health->Heal(DEBUG_HEAL_AMOUNT);
            }
        }
    }

    void DeveloperLevel::UpdateWavePanel()
    {
        if (hudScreen == nullptr)
        {
            return;
        }

        WaveHudState state;

        XYZEngine::GameObject* directorObject = level.GetWaveDirector();
        auto director = directorObject != nullptr ? directorObject->GetComponent<WaveDirectorComponent>() : nullptr;

        // Панель живёт ровно столько, сколько идёт осада: на пустой карте её быть не должно.
        if (director != nullptr && !director->IsCleared() && director->GetWavesCount() > 0)
        {
            state.isRunning = true;
            state.isPause = director->IsPause();
            state.current = director->GetCurrentWave() + 1;
            state.total = director->GetWavesCount();
            state.left = director->CountAliveNearby();
            state.nextIn = director->GetPauseLeft();

            if (state.isPause)
            {
                // В затишье номер показывает уже отбитые волны, а не ту, что ещё не пришла.
                state.current = state.current < 0 ? 0 : state.current;
            }
        }

        hudScreen->SetWaves(state);
    }

    void DeveloperLevel::UpdateChasePanel()
    {
        if (hudScreen == nullptr)
        {
            return;
        }

        ChaseHudState state;

        XYZEngine::GameObject* pursuitObject = level.GetPursuit();
        auto pursuit = pursuitObject != nullptr ? pursuitObject->GetComponent<PursuitComponent>() : nullptr;

        if (pursuit != nullptr)
        {
            state.isRunning = true;
            state.progress = pursuit->GetProgress();
            state.isClose = pursuit->IsCloseBehind();
        }

        hudScreen->SetChase(state);
    }

    void DeveloperLevel::Restart()
    {
        LOG_INFO("Level restarts");

        Stop();
        Start();
    }

    void DeveloperLevel::Stop()
    {
        LOG_INFO("Developer level is stopping");

        XYZEngine::FrameClock::Instance()->StopTimeEffects();

        for (auto sceneObject : {uiRoot, crosshair, music, player, particles})
        {
            if (sceneObject != nullptr)
            {
                GameWorld::Instance()->DestroyGameObject(sceneObject);
            }
        }
        uiRoot = nullptr;
        crosshair = nullptr;
        music = nullptr;
        player = nullptr;
        camera = nullptr;

        // Без этого после перезапуска StartCutscene считал бы, что сцена всё ещё идёт,
        // и ни одна сцена больше не запустилась бы - включая побег на машине.
        cutscene = nullptr;
        takenParts.clear();
        skidTrails[0] = nullptr;
        skidTrails[1] = nullptr;
        escapeSpeed = 0.f;
        arrivalTime = 0.f;
        boardTime = 0.f;
        particles = nullptr;

        level.Clear();

        XYZEngine::UiManager::Instance()->Clear();
        GameWorld::Instance()->Clear();

        hudScreen.reset();
        inventoryScreen.reset();
        messageScreen.reset();
        fadeScreen.reset();
    }

    void DeveloperLevel::SetPaused(bool isPaused)
    {
        // Звук глушит движок: сцена не знает про все источники.
        Engine::Instance()->SetPaused(isPaused);
        RenderSystem::Instance()->GetMainWindow().setMouseCursorVisible(isPaused);
        RenderSystem::Instance()->HoldMouse(!isPaused);

        UpdateOverlay();

        LOG_INFO(isPaused ? "Game paused" : "Game resumed");
    }

    void DeveloperLevel::ShowGameOver()
    {
        state = RunState::GameOver;
        UpdateOverlay();

        LOG_INFO("Game over");
    }

    void DeveloperLevel::UpdateOverlay()
    {
        if (messageScreen == nullptr)
        {
            return;
        }

        if (Engine::Instance()->IsPaused())
        {
            messageScreen->Show(PAUSE_TITLE, PAUSE_HINT);
        }
        else if (state == RunState::GameOver)
        {
            messageScreen->Show(GAME_OVER_TITLE, GAME_OVER_HINT);
        }
        else if (state == RunState::Victory)
        {
            messageScreen->Show(VICTORY_TITLE, VICTORY_HINT);
        }
        else
        {
            messageScreen->Hide();
        }
    }
}
