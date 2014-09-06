#include <fenv.h>

// #include <cstdlib> 
// #include <ctime> 
#include "window/window.h"
#include "rendering/sprite.h"
#include "rendering/line.h"
#include "rendering/model.h"
#include "rendering/string.h"
#include "rendering/fish.h"
#include "rendering/terrain.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"
#include "utils/lib.h"
#include "utils/timer.h"
#include "simulation/simulation.h"
#include "simulation/ball.h"
#include <sstream>
#include <IL/ilut.h>

class Accumulator
{
    float mAccumulator;
    float mBase;
    float mStep;
    bool mRunning;

public:
    Accumulator(float base,
                float step = 1.f):
        mAccumulator(0.f),
        mBase(base),
        mStep(step),
        mRunning(false)
    {}

    void reset()
    {
        mRunning = true;
        mAccumulator = mBase;
    }

    void update()
    {
        if (mRunning) {
            mAccumulator += mStep;
        }
    }
    void update(float dt)
    {
        if (mRunning) {
            mAccumulator += dt;
        }
    }

    void stop() { mRunning = false; }
    float getValue() const { return mAccumulator; }
    bool running() const { return mRunning; }
};

std::vector<sb::Fish> shoalOfFish;

void initializeSwarmModelSet(int size, std::shared_ptr<sb::Shader> textureShader)
{
    srand((unsigned)time(0));
    for (int i = 0; i < size; ++i)
    {
            sb::Fish fish("salamon.obj",
                     textureShader, gResourceMgr.getTexture("salamon2.jpg"));
            fish.setPosition(rand() % 20 - 10.0, rand() % 20 - 10.0, rand() % 20 - 10.0);
            fish.setVelocity((rand() % 50 - 25)/50.0, (rand() % 50 - 25)/50.0, (rand() % 50 - 25)/50.0);
            fish.setScale(0.8f);
            shoalOfFish.push_back(fish);
    }
} 

Vec3 mass_rule(sb::Fish fish)
{
    Vec3 m_centrum(0.f,0.f, 0.f);
    
    for(sb::Fish fish_it : shoalOfFish)
    {
            m_centrum = m_centrum + fish_it.getPosition();
    }
    m_centrum = m_centrum - fish.getPosition();   
    m_centrum = m_centrum / ( shoalOfFish.size() - 1.0);

    return (m_centrum - fish.getPosition()) / 10000;
}

double distance(Vec3 A, Vec3 B)
{
    return sqrt( (A.x - B.x)*(A.x - B.x) + (A.y - B.y)*(A.y - B.y) + (A.z - B.z)*(A.z - B.z));
}

Vec3 not_so_close_rule(sb::Fish fish)
{
    Vec3 closeness_vector(0.f,0.f, 0.f);
    
    for(sb::Fish fish_it : shoalOfFish)
    {
        if (distance(fish_it.getPosition(), fish.getPosition()) < 0.5) {
            closeness_vector = closeness_vector - (fish_it.getPosition() - fish.getPosition())/100; 
        }
    }

    return closeness_vector;   
}

Vec3 similar_velocity_rule(sb::Fish fish)
{
    Vec3 perceived_velocity(0.f,0.f, 0.f);

    for(sb::Fish fish_it : shoalOfFish)
    {
        perceived_velocity = perceived_velocity + fish_it.getVelocity(); 
    }

    perceived_velocity = perceived_velocity - fish.getVelocity();
    perceived_velocity = perceived_velocity / ( shoalOfFish.size() - 1);
    
    return (perceived_velocity - fish.getVelocity()) / 100;
}

Vec3 tend_to_place(sb::Fish fish, Vec3 place)
{
    return (place - fish.getPosition()) / 100000;
}

Vec3 not_so_fast(sb::Fish fish, float max_speed)
{
    if (distance(Vec3(0.f, 0.f, 0.f), fish.getPosition()) > max_speed)
    {
        return -fish.getVelocity() / 2;
    }
    return Vec3(0.f, 0.f, 0.f);
}

Vec3 avoid_predator(sb::Fish fish, Vec3 predator_position)
{
    Vec3 avoid_predator_velocity(0.f,0.f, 0.f);
    if (distance(predator_position, fish.getPosition()) < 3.0f)
    {
        return (fish.getPosition() - predator_position) * 0.005f;
    }
    return avoid_predator_velocity;
}


Vec3 calculateVelocity(sb::Fish& fish, Vec3 predator_position)
{
    Vec3 before = {0.f, 0.f, 1.f}; 
    Vec3 after = fish.getVelocity() + tend_to_place(fish, Vec3(0.f, 1.f, 1.f)) + mass_rule(fish)  + not_so_close_rule(fish) + similar_velocity_rule(fish) + avoid_predator(fish, predator_position);
    Vec3 rotation_axis = before.cross(fish.getVelocity());    
    float rotation_angle = acos(before.dot(fish.getVelocity()));
    
    if(! rotation_axis.isZero()){
        fish.setRotation(rotation_axis, Radians(rotation_angle));
    }
         
    return after;
}

int conuter = 0;

void animateFish(sb::Fish& fish)
{  
    // conuter++;
    // if (conuter % 7 == 0) {
    //     Vec3 source = fish.getRotationAxis();
    //     Vec3 dest = fish.getVelocity();
    //     Vec3 rotAxis = source.cross(dest).normalized();
    //     float rotAngle = acos(source.normalized().dot(dest.normalized())); 
    //     fish.setRotation(rotAxis, Radians(rotAngle/8));
    // }

}

Vec3 calculatePosition(sb::Fish fish)
{
    return fish.getVelocity() + fish.getPosition();
}

int main()
{
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);

    using sb::utils::makeString;

    sb::Window wnd(800, 600);

    auto colorShader = gResourceMgr.getShader("proj_basic.vert", "color.frag");
    auto textureShader = gResourceMgr.getShader("proj_texture.vert", "texture.frag");
    auto textureLightShader = gResourceMgr.getShader("proj_texture_normal.vert", "texture_normal.frag");

    wnd.setTitle("Sandbox");
    wnd.lockCursor();
    wnd.hideCursor();

    // Sim::Simulation sim(Sim::Simulation::SimSingleThrow,
    //                     textureLightShader,
    //                     colorShader);
    // sim.setThrowStart(Vec3d(0., 1., 0.), Vec3d(30., 30., 0.));

    // sb::Sprite crosshair("dot.png", textureShader);
    // crosshair.setPosition(0.f, 0.f, 0.f);
    // crosshair.setScale(0.01f, 0.01f * 1.33f, 1.f);

    // sb::Line xaxis(Vec3(1000.f, 0.f, 0.f),
    //                sb::Color(0.f, 0.f, 1.f),
    //                colorShader);
    // sb::Line yaxis(Vec3(0.f, 1000.f, 0.f),
    //                sb::Color(1.f, 0.f, 0.f),
    //                colorShader);
    // sb::Line zaxis(Vec3(0.f, 0.f, 1000.f),
    //                sb::Color(0.f, 1.f, 0.f),
    //                colorShader);

    sb::Model skybox("skybox.obj",
                     textureShader,
                     gResourceMgr.getTexture("miramar.jpg"));
    skybox.setScale(1000.f);

    // sb::Model shark("dragon.obj",
    //                  textureShader, gResourceMgr.getTexture("salamon.jpg"));
    // shark.setScale(3.f);

    initializeSwarmModelSet(150, textureShader);
    // sb::Model tower("sphere.obj",
    //                  textureShader,
    //                  gResourceMgr.getTexture("miramar.jpg"));
    // tower.setPosition(0.f, 0.f, 0.f);

    sb::Terrain terrain("hmap_flat.jpg", "ground.jpg", textureShader);
    terrain.setScale(10.f, 1.f, 10.f);
    terrain.setPosition(-640.f, -30.f, -640.f);

    gLog.info("all data loaded!\n");

    wnd.getCamera().lookAt(Vec3(5.f, 5.f, 20.f), Vec3(5.f, 5.f, 0.f));

    float moveSpeed = 0.f, strafeSpeed = 0.f, ascendSpeed = 0.f;
    const float SPEED = 0.5f;

    sb::Timer clock;
    float lastFrameTime = 0.f;
    // float physicsUpdateStep = 0.03f;
    float fpsCounter = 0.f;      // how many frames have been rendered in fpsUpdateStep time?
    float fpsUpdateStep = 1.f;   // update FPS-string every fpsUpdateStep seconds
    float fpsCurrValue = 0.f;    // current FPS value

    Accumulator deltaTime(0.f, 0.f);
    Accumulator fpsDeltaTime(0.f, 0.f);
    Accumulator throwVelocity(10.f, 0.5f);
    Accumulator windVelocity(0.5f, 0.5f);

    deltaTime.reset();
    fpsDeltaTime.reset();

    std::string fpsString;

    bool displayHelp = true;
    // bool displaySimInfo = true;
    // bool displayBallInfo = true;

    const std::string helpString =
        "controls:\n"
        "wasdqz + mouse - moving\n"
        "123456 - camera presets\n"
        "esc - exit\n"
        "f1 - show/hide controls info\n"
        "f2 - show/hide simulation info\n"
        "f3 - show/hide ball info\n"
        "f4 - show/hide ball launcher lines\n"
        "f8 - exit + display debug info\n"
        "print screen - save screenshot\n"
        "p - pause simulation\n"
        "o - switch vector display mode (forces/accelerations)\n"
        "i - enable/disable auto-pause when a ball hits ground\n"
        "t - reset simulation (single ball)\n"
        "r - reset simulation (multiple balls)\n"
        "mouse left - set throw position/initial velocity*\n"
        "mouse right - set wind force*\n"
        "cf - decrease/increase slow motion factor\n"
        "vg - decrease/increase line width\n"
        "bh - decrease/increase air density\n"
        "nj - decrease/increase ball path length\n"
        "mk - decrease/increase ball spawn delay\n"
        ",l - decrease/increase ball limit by 5\n"
        ".; - decrease/increase ball mass**\n"
        "/' - decrease/increase ball radius**\n"
        "* hold button to adjust value\n"
        "** doesn't affect existing balls";
    //const uint32_t helpStringLines = 27u;

    gLog.info("entering main loop\n");

    // main loop
    while(wnd.isOpened())
    {
        float delta = clock.getSecsElapsed() - lastFrameTime;
        deltaTime.update(delta);
        fpsDeltaTime.update(delta);
        ++fpsCounter;
        lastFrameTime = clock.getSecsElapsed();

        // FPS update
        if (fpsDeltaTime.getValue() >= fpsUpdateStep)
        {
            fpsCurrValue = fpsCounter / fpsUpdateStep;
            fpsString = "FPS = " + sb::utils::toString(fpsCurrValue);
            fpsDeltaTime.update(-fpsUpdateStep);
            fpsCounter = 0.f;
        }

        // event handling
        sb::Event e;
        while (wnd.getEvent(e))
        {
            switch (e.type)
            {
            case sb::Event::MousePressed:
                {
                    switch (e.data.mouse.button)
                    {
                    case sb::Mouse::ButtonLeft:
                        // if (!throwVelocity.running()) {
                        //     throwVelocity.reset();
                        // }
                        break;
                    case sb::Mouse::ButtonRight:
                        // if (!windVelocity.running()) {
                        //     windVelocity.reset();
                        // }
                        break;
                    default:
                        break;
                    }
                    break;
                }
            case sb::Event::MouseReleased:
                {
                    switch(e.data.mouse.button)
                    {
                    case sb::Mouse::ButtonLeft:
                        {
                            // Vec3 pos = wnd.getCamera().getEye();
                            // if (pos.y < sim.getBallRadius()) {
                            //     pos.y = (float)sim.getBallRadius();
                            // }

                            // Vec3 v = wnd.getCamera().getFront().normalized()
                            //                         * throwVelocity.getValue();
                            // sim.setThrowStart(Vec3d(pos), Vec3d(v));
                            // sim.reset();
                            // throwVelocity.stop();
                            break;
                        }
                        break;
                    case sb::Mouse::ButtonRight:
                        // sim.setWind(Vec3d(wnd.getCamera().getFront().normalized()
                        //                   * windVelocity.getValue()));
                        // windVelocity.stop();
                        break;
                    default:
                        break;
                    }
                    break;
                }
            case sb::Event::KeyPressed:
                {
                    switch (e.data.key)
                    {
                    case sb::Key::Num1:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(50.f, 0.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num2:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(-50.f, 0.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num3:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.0001f, 50.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num4:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.0001f, -50.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num5:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.f, 0.f, 50.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num6:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.f, 0.f, -50.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::N:
                        // sim.increaseBallPathLength(-1.0);
                        break;
                    case sb::Key::J:
                        // sim.increaseBallPathLength(1.0);
                        break;
                    case sb::Key::M:
                        // sim.increaseBallThrowDelay(-0.1f);
                        break;
                    case sb::Key::K:
                        // sim.increaseBallThrowDelay(0.1f);
                        break;
                    case sb::Key::B:
                        // sim.increaseAirDensity(-0.01);
                        break;
                    case sb::Key::H:
                        // sim.increaseAirDensity(0.01);
                        break;
                    case sb::Key::Comma:
                        // sim.increaseMaxBalls(-5);
                        break;
                    case sb::Key::L:
                        // sim.increaseMaxBalls(5);
                        break;
                    case sb::Key::A: strafeSpeed = -SPEED; break;
                    case sb::Key::D: strafeSpeed = SPEED; break;
                    case sb::Key::S: moveSpeed = -SPEED; break;
                    case sb::Key::W: moveSpeed = SPEED; break;
                    case sb::Key::Q: ascendSpeed = SPEED; break;
                    case sb::Key::Z: ascendSpeed = -SPEED; break;
                    case sb::Key::C:
                        // sim.increaseSloMoFactor(-0.1f);
                        break;
                    case sb::Key::F:
                        // sim.increaseSloMoFactor(0.1f);
                        break;
                    case sb::Key::Slash:
                        // sim.increaseBallRadius(-0.1f);
                        break;
                    case sb::Key::Apostrophe:
                        // sim.increaseBallRadius(0.1f);
                        break;
                    case sb::Key::Period:
                        // sim.increaseBallMass(-0.1f);
                        break;
                    case sb::Key::Colon:
                        // sim.increaseBallMass(0.1f);
                        break;
                    case sb::Key::Esc:
                        wnd.close();
                        break;
                    default:
                        break;
                    }
                    break;
                }
            case sb::Event::KeyReleased:
                {
                    switch (e.data.key)
                    {
                    case sb::Key::A:
                    case sb::Key::D:
                        strafeSpeed = 0.f;
                        break;
                    case sb::Key::S:
                    case sb::Key::W:
                        moveSpeed = 0.f;
                        break;
                    case sb::Key::Q:
                    case sb::Key::Z:
                        ascendSpeed = 0.f;
                        break;
                    case sb::Key::P:
                        // sim.togglePause();
                        break;
                    case sb::Key::O:
                        // sim.toggleVectorDisplayType();
                        break;
                    case sb::Key::I:
                        // sim.togglePauseOnGroundHit();
                        break;
                    case sb::Key::R:
                        // sim = Sim::Simulation(Sim::Simulation::SimContiniousThrow,
                        //                       textureShader,
                        //                       colorShader);
                        break;
                    case sb::Key::T:
                        // sim = Sim::Simulation(Sim::Simulation::SimSingleThrow,
                        //                       textureShader,
                        //                       colorShader);
                        break;
                    case sb::Key::V:
                        {
                            GLfloat lineWidth = 0.f;
                            GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
                            if (lineWidth > 1.f) {
                                GL_CHECK(glLineWidth(lineWidth / 2.f));
                            }
                            break;
                        }
                    case sb::Key::G:
                        {
                            GLfloat lineWidth = 0.f;
                            GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
                            if (lineWidth < 9.f) {
                                GL_CHECK(glLineWidth(lineWidth * 2.f));
                            }
                            break;
                        }
                    case sb::Key::F1:
                        displayHelp = !displayHelp;
                        break;
                    case sb::Key::F2:
                        // displaySimInfo = !displaySimInfo;
                        break;
                    case sb::Key::F3:
                        // displayBallInfo = !displayBallInfo;
                        break;
                    case sb::Key::F4:
                        // sim.toggleShowLauncherLines();
                        break;
                    case sb::Key::PrintScreen:
                        {
#ifdef PLATFORM_WIN32
                            SYSTEMTIME systime;
                            ::GetSystemTime(&systime);
                            std::string filename =
                                    makeString(systime.wHour, ".",
                                               systime.wMinute, ".",
                                               systime.wSecond, ".",
                                               systime.wMilliseconds, ".png");
#else // PLATFORM_LINUX
                            timeval current;
                            gettimeofday(&current, NULL);
                            std::string filename =
                                    makeString(current.tv_sec, ".",
                                               (current.tv_usec / 1000), ".png");
#endif // PLATFORM_WIN32

                            wnd.saveScreenshot(filename);
                            break;
                        }
                    default:
                        break;
                    }
                    break;
                }
            case sb::Event::MouseMoved:
                {
                    if (wnd.hasFocus())
                    {
                        Vec2i halfSize = wnd.getSize() / 2;
                        int pixelsDtX = (int)e.data.mouse.x - halfSize.x;
                        int pixelsDtY = (int)e.data.mouse.y - halfSize.y;

                        static const float ROTATION_SPEED = 1.0f;
                        Radians dtX = Radians(ROTATION_SPEED * (float)pixelsDtX / halfSize.x);
                        Radians dtY = Radians(ROTATION_SPEED * (float)pixelsDtY / halfSize.y);

                        wnd.getCamera().mouseLook(dtX, dtY);
                    }
                    break;
                }
            case sb::Event::WindowFocus:
                if (e.data.focus)
                {
                    wnd.hideCursor();
                    wnd.lockCursor();
                }
                else
                {
                    wnd.hideCursor(false);
                    wnd.lockCursor(false);
                    moveSpeed = strafeSpeed = 0.f;
                }
                break;
            case sb::Event::WindowResized:
                // crosshair.setScale(0.01f, 0.01f * ((float)e.data.wndResize.width / (float)e.data.wndResize.height), 0.01f);
                break;
            case sb::Event::WindowClosed:
                wnd.close();
                break;
            default:
                break;
            }
        }

        // physics update
        // uint32_t guard = 3u;
        // while ((deltaTime.getValue() >= physicsUpdateStep) && guard--)
        // {
        //     sim.update(physicsUpdateStep);
        //     deltaTime.update(-physicsUpdateStep);

        //     throwVelocity.update();
        //     windVelocity.update();
        // }

        // drawing
        wnd.getCamera().moveRelative(Vec3(strafeSpeed, ascendSpeed, moveSpeed));

        // move skybox, so player won't go out of it
        skybox.setPosition(wnd.getCamera().getEye());

        wnd.clear(sb::Color(0.f, 0.f, 0.5f));

        wnd.setAmbientLightColor(sb::Color(0.2f, 0.2f, 0.2f));
        wnd.addLight(sb::Light::point(Vec3(10.0, 10.0, 0.0), 100.0f));
        wnd.addLight(sb::Light::parallel(Vec3(5.0f, -10.0f, 5.0f), 100.0f));

        // environment
        wnd.draw(skybox);
        wnd.draw(terrain);
        

        // float h_angle = wnd.getCamera().getHorizontalAngle().value();
        // float v_angle = wnd.getCamera().getVerticalAngle().value();
        // shark.setPosition(wnd.getCamera().getEye());//, + Vec3(sin(h_angle) * 2.0, -0.5f + sin(v_angle), cos(h_angle) * 2.0));
        // shark.rotate(Vec3(0.0, 1.0, 0.0), Radians(v_angle/57.0));

        // wnd.draw(shark);

        for(sb::Fish &fish : shoalOfFish) {
            fish.setVelocity(calculateVelocity(fish, wnd.getCamera().getEye()));
            animateFish(fish);
        }

        for(sb::Fish &fish : shoalOfFish) {
            fish.setPosition(calculatePosition(fish));
            wnd.draw(fish);
        }        

        // axes - disable edpth test to prevent blinking
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest, false);
        // wnd.draw(xaxis);
        // wnd.draw(yaxis);
        // wnd.draw(zaxis);
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest);

        // balls & forces
        // sim.drawAll(wnd.getRenderer());

        // crosshair
        // wnd.draw(crosshair);

#if 0
        // info strings
        uint32_t nextLine = 0u;
        sb::String::print(fpsString, 0.f, 0.f,
                          (fpsCurrValue > 30.f
                              ? sb::Color::Green
                              : (fpsCurrValue > 20.f ? sb::Color::Yellow
                                                     : sb::Color::Red)),
                          nextLine++);
        sb::String::print(
                makeString("pos = ", wnd.getCamera().getEye(),
                           "\nfront = ", wnd.getCamera().getFront(),
                           " len = ", wnd.getCamera().getFront().length(),
                           "\nright = ", wnd.getCamera().getRight(),
                           "\nupReal = ", wnd.getCamera().getUpReal(),
                           "\nphi = ",
                           Degrees(wnd.getCamera().getHorizontalAngle()),
                           "\ntheta = ",
                           Degrees(wnd.getCamera().getVerticalAngle())),
                0.f, 0.f, sb::Color::White, nextLine);
        nextLine += 6;

        sb::String::print(makeString("throw velocity = ",
                                     throwVelocity.getValue()),
                           0.f, 0.f, Sim::ColorThrow, nextLine++);
        sb::String::print(makeString("wind velocity = ",
                                     windVelocity.getValue()),
                          0.f, 0.f, Sim::ColorWind, nextLine++);

        if (displayHelp)
        {
            sb::String::print(helpString,
                              0.f, 0.0f, sb::Color::White, ++nextLine);
            nextLine += helpStringLines;
        }
        if (displaySimInfo)
            nextLine = sim.printParametersToScreen(0.f, 0.f, ++nextLine);
        if (displayBallInfo)
            nextLine = sim.printBallParametersToScreen(
                    sim.raycast(wnd.getCamera().getEye(),
                                wnd.getCamera().getFront().normalized()),
                    0.f, 0.0f, nextLine);
#endif
        wnd.display();
    }

    gLog.info("window closed\n");
    return 0;
}

