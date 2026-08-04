#include "pch.h"
#include "Engine.h"
#include <iostream>

Engine::Engine() {}

void Engine::Initialize()
{
	std::cout << "Engine was init" << std::endl;
}

void Engine::Run()
{
	std::cout << "Engine is runing" << std::endl;
}
