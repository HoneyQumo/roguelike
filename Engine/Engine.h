#ifndef ENGINE_H 
#define ENGINE_H
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
class ENGINE_API Engine {
public:
	Engine();
	void Initialize();
	void Run();
};

#endif // ENGINE_H