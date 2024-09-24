#ifndef __LOGO_H__
#define __LOGO_H__

#include "Scene.hpp"
#include "Renderer.hpp"

class LogoScene : public Sputnik::Scene
{
public:
	void init() override;
	void returned() override;
	void update(float delta_time) override;
	void render(float delta_time) override;

private:
	Sputnik::Texture logo;
	float timer;
};
#endif // __LOGO_H__