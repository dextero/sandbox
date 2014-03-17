#include "window/window.h"
#include "utils/timer.h"
#include "utils/types.h"
#include "utils/libUtils.h"
#include "rendering/sprite.h"
#include "rendering/shader.h"
#include "rendering/sharedVertexBuffer.h"
#include "rendering/terrain.h"
#include <list>

class ParticleManager
{
public:
    enum ParticleType {
        ParticleInvalid = 0,
        ParticleTypeFirst,
        ParticleFire = ParticleTypeFirst,
        ParticleWater,

        ParticleTypesCount
    };

    struct ParticleBatch {
        std::vector<Vec2> position;
        std::vector<Vec2> velocity;
        std::vector<ParticleType> type;
        std::vector<float> timeToLive;
    };

    sb::SharedVertexBuffer mSharedVertexBuffer;

    std::vector<sb::TextureId> mTextures;
    std::map<ParticleType, ParticleBatch> mParticles;

    ParticleManager();

    void Remove(ParticleType type, size_t id);
    void Update(float dt);
    void Spawn(const Vec2& pos, ParticleType type);
    void DrawAll(sb::Window& wnd);
};

ParticleManager::ParticleManager()
{
    mTextures.resize(ParticleTypesCount);

    mTextures[ParticleInvalid] = gResourceMgr.GetTexture(L"error.png");
    mTextures[ParticleFire] = gResourceMgr.GetTexture(L"fire.png");
    mTextures[ParticleWater] = gResourceMgr.GetTexture(L"water.png");

    for (int type = ParticleTypeFirst; type < ParticleTypesCount; ++type)
    {
        ParticleBatch& batch = mParticles[(ParticleType)type];

        batch.position.reserve(1000);
        batch.velocity.reserve(1000);
        batch.type.reserve(1000);
        batch.timeToLive.reserve(1000);
    }
}

void ParticleManager::Remove(ParticleType type, size_t id)
{
    ParticleBatch& batch = mParticles[type];

    if (batch.position.size())
    {
        size_t last = batch.position.size() - 1;
        if (id != last)
        {
            batch.position[id] = batch.position[last];
            batch.velocity[id] = batch.velocity[last];
            batch.type[id] = batch.type[last];
            batch.timeToLive[id] = batch.timeToLive[last];
        }

        batch.position.pop_back();
        batch.velocity.pop_back();
        batch.type.pop_back();
        batch.timeToLive.pop_back();
    }
}

void ParticleManager::Update(float dt)
{
    for (std::map<ParticleType, ParticleBatch>::iterator it = mParticles.begin(); it != mParticles.end(); ++it)
    {
        ParticleBatch& batch = it->second;

        for (size_t i = 0; i < std::min(batch.position.size(), (size_t)100);)
        {
            if (batch.timeToLive[i] <= 0.f)
                Remove(it->first, i);
            else
            {
                batch.position[i] += batch.velocity[i] * dt;
                //batch.timeToLive[i] -= dt;
                ++i;
            }
        }
    }
}

void ParticleManager::Spawn(const Vec2& pos, ParticleType type)
{
    ParticleBatch& batch = mParticles[type];

    for (int i = 0; i < 20; ++i)
    {
        batch.position.push_back(pos);
        batch.velocity.push_back(Vec2((float)(rand() % 11) - 5.f, (float)(rand() % 11) - 5.f) / 100.f);
        batch.type.push_back(type);
        batch.timeToLive.push_back(10.f);
    }
}

void ParticleManager::DrawAll(sb::Window& wnd)
{
    GL_CHECK(glEnable(GL_TEXTURE_2D));
    glPointSize(5.f);

    for (std::map<ParticleType, ParticleBatch>::iterator it = mParticles.begin(); it != mParticles.end(); ++it)
    {
        ParticleBatch& batch = it->second;

        if (batch.position.size() > 0)
        {
            uint offset = mSharedVertexBuffer.AddVertices(&batch.position[0], NULL, NULL, batch.position.size());
            mSharedVertexBuffer.Bind();

            sb::Shader::Use(sb::Shader::ShaderPointSprite);
            sb::Shader& shader = sb::Shader::GetCurrent();
            static Mat44 identity = Mat44();

            shader.SetUniform("u_matViewProjection", wnd.GetCamera().GetOrthographicProjectionMatrix());
            shader.SetUniform("u_matModel", identity);
            shader.SetUniform("u_color", sb::Color::White);
            shader.SetUniform("u_texture", (int)sb::Shader::SamplerImage);

            GL_CHECK(glActiveTexture(GL_TEXTURE0));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTextures[it->first]));

            GL_CHECK(glVertexPointer(2, GL_FLOAT, 0, (void*)offset));
            GL_CHECK(glDrawArrays(GL_POINTS, 0, batch.position.size()));

            mSharedVertexBuffer.Unbind();
            mSharedVertexBuffer.ReleaseVertices(offset);
        }
    }
}


int main()
{
    sb::Window window(800, 600);
    window.GetRenderer().EnableFeature(sb::Renderer::FeatureBackfaceCulling, false);

    Vec2 halfWndSize = Vec2(window.GetSize()) / 2.f;
    sb::Timer timer;

    ParticleManager particles;

    sb::Event event;
    while (window.IsOpened())
    {
        while (window.GetEvent(event))
        {
            switch (event.type)
            {
            case sb::Event::WindowClosed:
                window.Close();
                break;
            case sb::Event::KeyPressed:
                switch (event.data.key)
                {
                case sb::Key::Esc:
                    window.Close();
                    break;
                default:
                    break;
                }
                break;
            case sb::Event::MousePressed:
                {
                    ParticleManager::ParticleType type;
                    switch (event.data.mouse.button)
                    {
                    case sb::Mouse::ButtonLeft:
                        type = ParticleManager::ParticleFire;
                        break;
                    case sb::Mouse::ButtonRight:
                        type = ParticleManager::ParticleWater;
                        break;
                    default:
                        type = ParticleManager::ParticleInvalid;
                        break;
                    }

                    if (type != ParticleManager::ParticleInvalid)
                        particles.Spawn(Vec2((float)event.data.mouse.x / halfWndSize[0] - 1.f, -((float)event.data.mouse.y / halfWndSize[1] - 1.f)), type);
                    break;
                }
            default:
                break;
            }
        }

        particles.Update(timer.GetSecsElapsed());
        timer.Reset();

        window.Clear(sb::Color::Black);
            particles.DrawAll(window);
        window.Display();
    }

    return 0;
}
