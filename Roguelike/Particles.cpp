#include "Particles.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ParticleSystemComponent.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateParticles()
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(PARTICLES_OBJECT_NAME);
        gameObject->SetRenderLayer(EFFECT_RENDER_LAYER);

        auto particles = gameObject->AddComponent<XYZEngine::ParticleSystemComponent>();
        particles->SetCapacity(PARTICLE_POOL_CAPACITY);

        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(FX_ATLAS_TEXTURE);
        if (texture == nullptr)
        {
            LOG_ERROR("Fx atlas texture is not loaded, particles stay invisible");
        }
        else
        {
            particles->SetTexture(texture);
        }

        LOG_INFO("Particles created");
        return gameObject;
    }
}
