/*
#include "window/window.h"
#include "rendering/sprite.h"
#include "rendering/line.h"
#include "rendering/model.h"
#include "rendering/string.h"
#include "rendering/terrain.h"
#include "utils/allocator.h"
#include "utils/profiler.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"
#include "utils/libUtils.h"
#include "simulation/simulation.h"
#include "simulation/ball.h"
#include <sstream>
#include <IL/ilut.h>

void wait_for_key()
{
	getchar();
}

void print_reports()
{
	PRINT_MEMLEAK_REPORT(L"memleak.txt");
	PRINT_PROFILE_REPORT("profile.txt");
}

class Accumulator
{
	float mAccumulator,
		  mBase,
		  mStep;
	bool mRunning;

public:
	Accumulator(float base, float step = 1.f): mAccumulator(0.f), mBase(base), mStep(step), mRunning(false) {}
	void Reset() { mRunning = true; mAccumulator = mBase; }
	void Stop() { mRunning = false; }
	void Update() { if (mRunning) mAccumulator += mStep; }
	void Update(float dt) { if (mRunning) mAccumulator += dt; }
	float GetValue() const { return mAccumulator; }
	bool Running() const { return mRunning; }
};

int main()
{
	PROFILE();

	sb::Window wnd(1200, 900);
	wnd.SetTitle("Sandbox");
	wnd.LockCursor();
	wnd.HideCursor();

	Sim::Simulation sim(Sim::Simulation::SimSingleThrow);
	sim.SetThrowStart(Vec3d(0., 1., 0.), Vec3d(30., 30., 0.));

	sb::Sprite crosshair(L"dot.png");
	crosshair.SetPosition(0.f, 0.f, 0.f);
	crosshair.SetScale(0.01f, 0.01f * 1.33f, 1.f);

	sb::Line xaxis(Vec3(1000.f, 0.f, 0.f), sb::Color(0.f, 0.f, 1.f)),
		 yaxis(Vec3(0.f, 1000.f, 0.f), sb::Color(1.f, 0.f, 0.f)),
		 zaxis(Vec3(0.f, 0.f, 1000.f), sb::Color(0.f, 1.f, 0.f));

	sb::Model skybox(L"skybox.obj");
	skybox.SetScale(1000.f);

	sb::Terrain terrain(L"hmap_flat.jpg", L"ground.jpg");
	terrain.SetScale(10.f, 1.f, 10.f);
	terrain.SetPosition(-640.f, 0.f, -640.f);

	gLog.Info("all data loaded!\n");

	wnd.GetCamera().LookAt(Vec3(5.f, 5.f, 20.f), Vec3(5.f, 5.f, 0.f));

	float angleX = 0.f, angleY = 0.f;
	float moveSpeed = 0.f, strafeSpeed = 0.f, ascendSpeed = 0.f;
	const float SPEED = 0.5f;

	sb::Timer clock;
	float lastFrameTime = 0.f,
		  physicsUpdateStep = 0.03f,
		  fpsCounter = 0.f,		// how many frames have been rendered in fpsUpdateStep time?
		  fpsUpdateStep = 1.f,	// update FPS-string every fpsUpdateStep seconds
		  fpsCurrValue = 0.f;	// current FPS value

	Accumulator deltaTime(0.f, 0.f),
				fpsDeltaTime(0.f, 0.f),
				throwVelocity(10.f, 0.5f),
				windVelocity(0.5f, 0.5f);
	deltaTime.Reset();
	fpsDeltaTime.Reset();

	std::string fpsString;

	bool displayHelp = true, displaySimInfo = true, displayBallInfo = true;
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
	const uint helpStringLines = 27u;

	gLog.Info("entering main loop\n");

	// main loop
	while(wnd.IsOpened())
	{
		float delta = clock.GetSecsElapsed() - lastFrameTime;
		deltaTime.Update(delta);
		fpsDeltaTime.Update(delta);
		++fpsCounter;
		lastFrameTime = clock.GetSecsElapsed();

		// FPS update
		if (fpsDeltaTime.GetValue() >= fpsUpdateStep)
		{
			fpsCurrValue = fpsCounter / fpsUpdateStep;
			fpsString = "FPS = " + sb::StringUtils::ToString(fpsCurrValue);
			fpsDeltaTime.Update(-fpsUpdateStep);
			fpsCounter = 0.f;
		}

		// event handling
		sb::Event e;
		while (wnd.GetEvent(e))
		{
			switch (e.type)
			{
			case sb::Event::MousePressed:
				{
					switch (e.data.mouse.button)
					{
					case sb::Mouse::ButtonLeft:
						if (!throwVelocity.Running())
							throwVelocity.Reset();
						break;
					case sb::Mouse::ButtonRight:
						if (!windVelocity.Running())
							windVelocity.Reset();
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
							Vec3 pos = wnd.GetCamera().GetEye();
							if (pos[1] < sim.mBallRadius)
								pos[1] = (float)sim.mBallRadius;

							Vec3 v = wnd.GetCamera().GetFront().normalized() * throwVelocity.GetValue();
							sim.SetThrowStart(Vec3d(pos[0], pos[1], pos[2]), Vec3d(v[0], v[1], v[2]));
							sim.Reset();
							throwVelocity.Stop();
							break;
						}
						break;
					case sb::Mouse::ButtonRight:
						sim.SetWind(wnd.GetCamera().GetFront().normalized() * windVelocity.GetValue());
						windVelocity.Stop();
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
						wnd.GetCamera().LookAt(Vec3(50.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f));
						break;
					case sb::Key::Num2:
						moveSpeed = strafeSpeed = 0.f;
						wnd.GetCamera().LookAt(Vec3(-50.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f));
						break;
					case sb::Key::Num3:
						moveSpeed = strafeSpeed = 0.f;
						wnd.GetCamera().LookAt(Vec3(0.0001f, 50.f, 0.f), Vec3(0.f, 0.f, 0.f));
						break;
					case sb::Key::Num4:
						moveSpeed = strafeSpeed = 0.f;
						wnd.GetCamera().LookAt(Vec3(0.0001f, -50.f, 0.f), Vec3(0.f, 0.f, 0.f));
						break;
					case sb::Key::Num5:
						moveSpeed = strafeSpeed = 0.f;
						wnd.GetCamera().LookAt(Vec3(0.f, 0.f, 50.f), Vec3(0.f, 0.f, 0.f));
						break;
					case sb::Key::Num6:
						moveSpeed = strafeSpeed = 0.f;
						wnd.GetCamera().LookAt(Vec3(0.f, 0.f, -50.f), Vec3(0.f, 0.f, 0.f));
						break;
					case sb::Key::N:
						sim.mBallPathLength = sb::Math::Clamp(sim.mBallPathLength - 1.0, 0.0, 1000.0);
						break;
					case sb::Key::J:
						sim.mBallPathLength = sb::Math::Clamp(sim.mBallPathLength + 1.0, 0.0, 1000.0);
						break;
					case sb::Key::M:
						sim.mBallThrowDelay -= 0.1f;
						break;
					case sb::Key::K:
						sim.mBallThrowDelay += 0.1f;
						break;
					case sb::Key::B:
						sim.mAirDensity -= 0.01;
						break;
					case sb::Key::H:
						sim.mAirDensity += 0.01;
						break;
					case sb::Key::Comma:
						if (sim.mSimType == Sim::Simulation::SimContiniousThrow && sim.mMaxBalls > 1u)
							sim.mMaxBalls -= 5;
						break;
					case sb::Key::L:
						if (sim.mSimType == Sim::Simulation::SimContiniousThrow)
							sim.mMaxBalls += 5;
						break;
					case sb::Key::A: strafeSpeed = -SPEED; break;
					case sb::Key::D: strafeSpeed = SPEED; break;
					case sb::Key::S: moveSpeed = -SPEED; break;
					case sb::Key::W: moveSpeed = SPEED; break;
					case sb::Key::Q: ascendSpeed = SPEED; break;
					case sb::Key::Z: ascendSpeed = -SPEED; break;
					case sb::Key::C:
						sim.mSloMoFactor -= 0.1f;
						break;
					case sb::Key::F:
						sim.mSloMoFactor += 0.1f;
						break;
					case sb::Key::Slash:
						sim.mBallRadius -= 0.1f;
						break;
					case sb::Key::Apostrophe:
						sim.mBallRadius += 0.1f;
						break;
					case sb::Key::Period:
						sim.mBallMass -= 0.1f;
						break;
					case sb::Key::Colon:
						sim.mBallMass += 0.1f;
						break;
					case sb::Key::Esc:
						wnd.Close();
						break;
					case sb::Key::F8:
						atexit(wait_for_key);
						atexit(print_reports);
						wnd.Close();
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
						sim.TogglePause();
						break;
					case sb::Key::O:
						sim.mVectorDisplayType = (sim.mVectorDisplayType == Sim::Simulation::DisplayForce ? Sim::Simulation::DisplayAcceleration : Sim::Simulation::DisplayForce);
						break;
					case sb::Key::I:
						sim.mPauseOnGroundHit = !sim.mPauseOnGroundHit;
						break;
					case sb::Key::R:
						sim = Sim::Simulation(Sim::Simulation::SimContiniousThrow);
						break;
					case sb::Key::T:
						sim = Sim::Simulation(Sim::Simulation::SimSingleThrow);
						break;
					case sb::Key::V:
						{
							GLfloat lineWidth = 0.f;
							GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
							if (lineWidth > 1.f)
								GL_CHECK(glLineWidth(lineWidth / 2.f));
							break;
						}
					case sb::Key::G:
						{
							GLfloat lineWidth = 0.f;
							GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
							if (lineWidth < 9.f)
								GL_CHECK(glLineWidth(lineWidth * 2.f));
							break;
						}
					case sb::Key::F1:
						displayHelp = !displayHelp;
						break;
					case sb::Key::F2:
						displaySimInfo = !displaySimInfo;
						break;
					case sb::Key::F3:
						displayBallInfo = !displayBallInfo;
						break;
					case sb::Key::F4:
						sim.mShowLauncherLines = !sim.mShowLauncherLines;
						break;
					case sb::Key::PrintScreen:
						{
							std::wstringstream filename;
#ifdef PLATFORM_WIN32
							SYSTEMTIME systime;
							::GetSystemTime(&systime);
							filename << systime.wHour << "." << systime.wMinute << "." << systime.wSecond << "." << systime.wMilliseconds << ".png";
#else // PLATFORM_LINUX
							timeval current;
							gettimeofday(&current, NULL);
							filename << current.tv_sec << "." << (current.tv_usec / 1000) << ".png";
#endif // PLATFORM_WIN32

							wnd.SaveScreenshot(filename.str().c_str());
							break;
						}
					default:
						break;
					}
					break;
				}
			case sb::Event::MouseMoved:
				{
					if (wnd.HasFocus())
					{
						float dtX = (float)((int)e.data.mouse.x - wnd.GetSize()[0] / 2) / (float)(wnd.GetSize()[0] / 2);
						float dtY = (float)((int)e.data.mouse.y - wnd.GetSize()[1] / 2) / (float)(wnd.GetSize()[1] / 2);
	
						wnd.GetCamera().MouseLook(dtX, dtY);
					}
					break;
				}
			case sb::Event::WindowFocus:
				if (e.data.focus)
				{
					wnd.HideCursor();
					wnd.LockCursor();
				}
				else
				{
					wnd.HideCursor(false);
					wnd.LockCursor(false);
					moveSpeed = strafeSpeed = 0.f;
				}
				break;
			case sb::Event::WindowResized:
				crosshair.SetScale(0.01f, 0.01f * ((float)e.data.wndResize.width / (float)e.data.wndResize.height), 0.01f);
				break;
			case sb::Event::WindowClosed:
				wnd.Close();
				break;
			default:
				break;
			}
		}

		// physics update
		uint guard = 3u;
		while ((deltaTime.GetValue() >= physicsUpdateStep) && guard--)
		{
			sim.Update(physicsUpdateStep);
			deltaTime.Update(-physicsUpdateStep);

			throwVelocity.Update();
			windVelocity.Update();
		}

		// drawing
		wnd.GetCamera().MoveRelative(Vec3(strafeSpeed, ascendSpeed, moveSpeed));

		// move skybox, so player won't go out of it
		skybox.SetPosition(wnd.GetCamera().GetEye());

		wnd.Clear(sb::Color(0.f, 0.f, 0.5f));

		// environment
		wnd.Draw(skybox);
		wnd.Draw(terrain);

		// axes - disable edpth test to prevent blinking
		wnd.GetRenderer().EnableFeature(sb::Renderer::FeatureDepthTest, false);
		wnd.Draw(xaxis);
		wnd.Draw(yaxis);
		wnd.Draw(zaxis);
		wnd.GetRenderer().EnableFeature(sb::Renderer::FeatureDepthTest);

		// balls & forces
		sim.DrawAll(wnd.GetRenderer());

		// crosshair
		wnd.Draw(crosshair);

		// info strings
		uint nextLine = 0u;
		sb::String::Print(fpsString, 0.f, 0.f, (fpsCurrValue > 30.f ? sb::Color::Green : (fpsCurrValue > 20.f ? sb::Color::Yellow : sb::Color::Red)), nextLine++);
		sb::String::Print("pos = " + sb::StringUtils::ToString(wnd.GetCamera().GetEye()) +
					  "\nfront = " + sb::StringUtils::ToString(wnd.GetCamera().GetFront()) +
					  "\nphi = " + sb::StringUtils::ToString(wnd.GetCamera().GetHorizontalAngle() * 180.f / PI) + " deg"
					  "\ntheta = " + sb::StringUtils::ToString(wnd.GetCamera().GetVerticalAngle() * 180.f / PI) + " deg", 0.f, 0.f, sb::Color::White, nextLine);
		nextLine += 4;
		sb::String::Print("throw velocity = " + sb::StringUtils::ToString(throwVelocity.GetValue()), 0.f, 0.f, Sim::ColorThrow, nextLine++);
		sb::String::Print("wind velocity = " + sb::StringUtils::ToString(windVelocity.GetValue()), 0.f, 0.f, Sim::ColorWind, nextLine++);

		if (displayHelp)
		{
			sb::String::Print(helpString, 0.f, 0.0f, sb::Color::White, ++nextLine);
			nextLine += helpStringLines;
		}
		if (displaySimInfo)
			nextLine = sim.PrintParametersToScreen(0.f, 0.f, ++nextLine);
		if (displayBallInfo)
			nextLine = sim.PrintBallParametersToScreen(sim.Raycast(wnd.GetCamera().GetEye(), wnd.GetCamera().GetFront().normalized()), 0.f, 0.0f, nextLine);

		wnd.Display();
	}

	gLog.Info("window closed\n");
	return 0;
}

*/