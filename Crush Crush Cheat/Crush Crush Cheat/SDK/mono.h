#pragma once
#include "../includes.hpp"

typedef MonoThread* (*t_mono_thread_attach)(MonoDomain*);
typedef MonoDomain* (*t_mono_get_root_domain)();
typedef MonoAssembly* (*t_mono_domain_assembly_open)(MonoDomain* doamin, const char* name);
typedef MonoImage* (*t_mono_assembly_get_image)(MonoAssembly* assembly);
typedef MonoClass* (*t_mono_class_from_name)(MonoImage* image, const char* name_space, const char* name);
typedef MonoMethod* (*t_mono_class_get_method_from_name)(MonoClass* klass, const char* name, int param_count);
typedef void* (*t_mono_compile_method)(MonoMethod* method);

class Mono
{
private:
	Mono() {};
	bool initalized = false;
	HMODULE hMono = nullptr;

	t_mono_thread_attach mono_thread_attach = nullptr;
	t_mono_get_root_domain mono_get_root_domain = nullptr;
	t_mono_domain_assembly_open mono_domain_assembly_open = nullptr;
	t_mono_assembly_get_image mono_assembly_get_image = nullptr;
	t_mono_class_from_name mono_class_from_name = nullptr;
	t_mono_class_get_method_from_name mono_class_get_method_from_name = nullptr;
	t_mono_compile_method mono_compile_method = nullptr;

	void Initalize()
	{
		std::cout << "Mono Initalizing!\n";
		hMono = GetModuleHandleA("mono.dll");
		if (hMono == NULL)
		{
			std::cout << "hMono was NULL!\n";
			return;
		}

		// Necessary functions to get method addresses
		mono_domain_assembly_open = reinterpret_cast<t_mono_domain_assembly_open>(GetProcAddress(hMono, "mono_domain_assembly_open"));
		mono_assembly_get_image = reinterpret_cast<t_mono_assembly_get_image>(GetProcAddress(hMono, "mono_assembly_get_image"));
		mono_class_from_name = reinterpret_cast<t_mono_class_from_name>(GetProcAddress(hMono, "mono_class_from_name"));
		mono_class_get_method_from_name = reinterpret_cast<t_mono_class_get_method_from_name>(GetProcAddress(hMono, "mono_class_get_method_from_name"));
		mono_compile_method = reinterpret_cast<t_mono_compile_method>(GetProcAddress(hMono, "mono_compile_method"));

		// Attach thread to prevent crashes
		mono_thread_attach = reinterpret_cast<t_mono_thread_attach>(GetProcAddress(hMono, "mono_thread_attach"));
		mono_get_root_domain = reinterpret_cast<t_mono_get_root_domain>(GetProcAddress(hMono, "mono_get_root_domain"));
		mono_thread_attach(mono_get_root_domain());

		std::cout << "Mono Initalized!\n";
		initalized = true;
	}

public:
	static Mono& instance()
	{
		static Mono _instance;

		if (!_instance.initalized)
			_instance.Initalize();

		return _instance;
	}

	void* GetMethod(const char* className, const char* methodName, int param_count = 0, const char* assemblyName = "Assembly-CSharp")
	{
		std::cout << "Getting Method!\n";

		MonoDomain* pDomain = mono_get_root_domain();
		if (pDomain == nullptr)
		{
			std::cout << "pDomain Null\n";
			return nullptr;
		}

		MonoAssembly* pAssembly = mono_domain_assembly_open(pDomain, assemblyName);
		if (pAssembly == nullptr)
		{
			std::cout << "pAssembly Null\n";
			return nullptr;
		}

		MonoImage* pImage = mono_assembly_get_image(pAssembly);
		if (pImage == nullptr)
		{
			std::cout << "pImage Null\n";
			return nullptr;
		}

		MonoClass* pKlass = mono_class_from_name(pImage, "", className);
		if (pKlass == nullptr)
		{
			std::cout << "pKlass Null\n";
			return nullptr;
		}
		
		MonoMethod* pMethod = mono_class_get_method_from_name(pKlass, methodName, param_count);
		if (pMethod == nullptr)
		{
			std::cout << "pMethod Null\n";
			return nullptr;
		}

		std::cout << "pMethod: " << std::hex << pMethod << std::dec << "\n";
		return mono_compile_method(pMethod);
	}
};