#include "MetaObject/MetaObjectFactory.hpp"
#include "RuntimeObjectSystem.h"
#include "IObjectFactorySystem.h"
#include "MetaObject/Logging/CompileLogger.hpp"
#include "MetaObject/IMetaObject.hpp"
#include "MetaObject/Logging/Log.hpp"
#include "MetaObject/Signals/TypedSignal.hpp"
#include "MetaObject/Signals/TypedSlot.hpp"
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
using namespace mo;

struct MetaObjectFactory::impl
{
    impl(SystemTable* table)
    {
        obj_system.Initialise(&logger, table);
    }
    RuntimeObjectSystem obj_system;
    CompileLogger logger;
    std::vector<std::string> plugins;
    TypedSignal<void(void)> on_constructor_added;
};

MetaObjectFactory::MetaObjectFactory(SystemTable* table)
{
    _pimpl = new impl(table);
}

MetaObjectFactory::~MetaObjectFactory()
{
    delete _pimpl;
}

IRuntimeObjectSystem* MetaObjectFactory::GetObjectSystem()
{
    return &_pimpl->obj_system;
}

MetaObjectFactory* MetaObjectFactory::Instance(SystemTable* table)
{
    static MetaObjectFactory g_inst(table);
    return &g_inst;
}

IMetaObject* MetaObjectFactory::Create(const char* type_name, int interface_id)
{
    auto constructor = _pimpl->obj_system.GetObjectFactorySystem()->GetConstructor(type_name);
    if(constructor)
    {
        if(interface_id != -1)
        {
            if(constructor->GetInterfaceId() != interface_id)
                return nullptr;
        }
        IObject* obj = constructor->Construct();
        if(IMetaObject* mobj = dynamic_cast<IMetaObject*>(obj))
        {
            mobj->Init(true);
            return mobj;
        }else
        {
            delete obj;
        }
    }
    return nullptr;
}

std::vector<std::string> MetaObjectFactory::ListConstructableObjects(int interface_id) const
{
    std::vector<std::string> output;
    AUDynArray<IObjectConstructor*> constructors;
    _pimpl->obj_system.GetObjectFactorySystem()->GetAll(constructors);
    for(int i = 0; i < constructors.Size(); ++i)
    {
        if(interface_id == -1)
            output.emplace_back(constructors[i]->GetName());
        else
            if(constructors[i]->GetInterfaceId() == interface_id)
                output.emplace_back(constructors[i]->GetName());
    }
    return output;
}

std::string MetaObjectFactory::PrintAllObjectInfo(int interface_id) const
{
    std::stringstream ss;
    AUDynArray<IObjectConstructor*> constructors;
    _pimpl->obj_system.GetObjectFactorySystem()->GetAll(constructors);
    for(int i = 0; i < constructors.Size(); ++i)
    {
        if(auto info = constructors[i]->GetObjectInfo())
        {
            if(interface_id == -1)
            {
                ss << info->Print();
            }
            else
            {
                if(constructors[i]->GetInterfaceId() == interface_id)
                {
                    ss << info->Print();
                }
            }
        }
    }
    return ss.str();
}

IObjectConstructor* MetaObjectFactory::GetConstructor(const char* type_name) const
{
    return _pimpl->obj_system.GetObjectFactorySystem()->GetConstructor(type_name);
}


std::vector<IObjectConstructor*> MetaObjectFactory::GetConstructors(int interface_id) const
{
    std::vector<IObjectConstructor*> output;
    AUDynArray<IObjectConstructor*> constructors;
    _pimpl->obj_system.GetObjectFactorySystem()->GetAll(constructors);
    for(int i = 0; i < constructors.Size(); ++i)
    {
        if(interface_id == -1)
            output.emplace_back(constructors[i]);
        else
            if(constructors[i]->GetInterfaceId() == interface_id)
                output.emplace_back(constructors[i]);
    }
    return output;
}

IObjectInfo* MetaObjectFactory::GetObjectInfo(const char* type_name) const
{
    auto constructor = _pimpl->obj_system.GetObjectFactorySystem()->GetConstructor(type_name);
    if(constructor)
    {
        return constructor->GetObjectInfo();
    }
    return nullptr;
}
std::vector<IObjectInfo*> MetaObjectFactory::GetAllObjectInfo() const
{
    std::vector<IObjectInfo*> output;
    AUDynArray<IObjectConstructor*> constructors;
    _pimpl->obj_system.GetObjectFactorySystem()->GetAll(constructors);
    for(int i = 0; i < constructors.Size(); ++i)
    {
        output.push_back(constructors[i]->GetObjectInfo());
    }
    return output;
}
void MetaObjectFactory::SetupObjectConstructors(IPerModuleInterface* pPerModuleInterface)
{
    GetObjectSystem()->SetupObjectConstructors(pPerModuleInterface);
}


std::vector<std::string> MetaObjectFactory::ListLoadedPlugins() const
{
    return _pimpl->plugins;
}
int MetaObjectFactory::LoadPlugins(const std::string& path_)
{
    boost::filesystem::path path(boost::filesystem::current_path().string() + "/" + path_);
    int count = 0;
    for(boost::filesystem::directory_iterator itr(path); itr != boost::filesystem::directory_iterator(); ++itr)
    {
#ifdef _MSC_VER
        if(itr->path().extension().string() == ".dll")
#else
        if(itr->path().extension().string() == ".so")
#endif
        {
            if(LoadPlugin(itr->path().string()))
                ++count;
        }
    }
    return count;
}

#ifdef _MSC_VER
#include "Windows.h"
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

bool MetaObjectFactory::LoadPlugin(const std::string& fullPluginPath)
{
    static int projectCount = 0;
    LOG(info) << "Loading plugin " << fullPluginPath;
    if (!boost::filesystem::is_regular_file(fullPluginPath))
    {
        return false;
    }
    std::string plugin_name = boost::filesystem::path(fullPluginPath).stem().string();
    HMODULE handle = LoadLibrary(fullPluginPath.c_str());
    if (handle == nullptr)
    {
        auto err = GetLastError();
        LOG(debug) << "Failed to load " << plugin_name << " due to: [" << err << "] " << GetLastErrorAsString();
        _pimpl->plugins.push_back(fullPluginPath + " - failed");
        return false;
    }
    typedef int(*BuildLevelFunctor)();
    BuildLevelFunctor buildLevel = (BuildLevelFunctor)GetProcAddress(handle, "GetBuildType");
    if (buildLevel)
    {
        /*if (buildLevel() != BUILD_TYPE)
        {
            LOG(debug) << "Library debug level does not match";
            _pimpl->plugins.push_back(fullPluginPath + " - failed");
            return false;
        }*/
    }
    else
    {
        LOG(debug) << "Build level not defined in library, attempting to load anyways";
    }

    typedef void(*includeFunctor)();
    includeFunctor functor = (includeFunctor)GetProcAddress(handle, "SetupIncludes");
    if (functor)
        functor();
    else
        LOG(debug) << "Setup Includes not found in plugin " << plugin_name;

    typedef IPerModuleInterface* (*moduleFunctor)();
    moduleFunctor module = (moduleFunctor)GetProcAddress(handle, "GetPerModuleInterface");
    if (module)
    {
        auto moduleInterface = module();
        SetupObjectConstructors(moduleInterface);
    }

    else
    {
        LOG(debug) << "GetPerModuleInterface not found in plugin " << plugin_name;
        module = (moduleFunctor)GetProcAddress(handle, "GetModule");
        if (module)
        {
            auto moduleInterface = module();
            SetupObjectConstructors(moduleInterface);
        }
        else
        {
            LOG(debug) << "GetModule not found in plugin " << plugin_name;
            FreeLibrary(handle);
        }
    }
    _pimpl->plugins.push_back(plugin_name + " - success");
    return true;
}
#else
#include "dlfcn.h"

bool MetaObjectFactory::LoadPlugin(const std::string& fullPluginPath)
{
    void* handle = dlopen(fullPluginPath.c_str(), RTLD_LAZY);
    // Fallback on old module

    typedef IPerModuleInterface* (*moduleFunctor)();

    moduleFunctor module = (moduleFunctor)dlsym(handle, "GetPerModuleInterface");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        LOG(debug)  << dlsym_error << '\n';
        module = (moduleFunctor)dlsym(handle, "GetModule");
        dlsym_error = dlerror();
        if (dlsym_error)
        {
            LOG(debug) << dlsym_error << '\n';
            return false;
        }
        LOG(debug) << "Found symbol 'GetModule'" << std::endl;

    }
    if (module == nullptr)
    {
        LOG(debug)  << "module == nullptr" << std::endl;
        return false;
    }
    SetupObjectConstructors(module());
    typedef void(*includeFunctor)();
    includeFunctor functor = (includeFunctor)dlsym(handle, "SetupIncludes");
    if (functor)
        functor();

    return true;
}



#endif

bool MetaObjectFactory::AbortCompilation()
{
	return _pimpl->obj_system.AbortCompilation();
}
bool MetaObjectFactory::CheckCompile()
{
	static boost::posix_time::ptime prevTime = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::time_duration delta = currentTime - prevTime;
	if (delta.total_milliseconds() < 10)
		return false;
	_pimpl->obj_system.GetFileChangeNotifier()->Update(float(delta.total_milliseconds()) / 1000.0);
	return IsCurrentlyCompiling();
}
bool MetaObjectFactory::IsCurrentlyCompiling()
{
	return _pimpl->obj_system.GetIsCompiling();
}
bool MetaObjectFactory::IsCompileComplete()
{
	return _pimpl->obj_system.GetIsCompiledComplete();
}
bool MetaObjectFactory::SwapObjects()
{
	if (IsCompileComplete())
	{
		return _pimpl->obj_system.LoadCompiledModule();
	}
	return false;
}
void MetaObjectFactory::SetCompileCallback(std::function<void(const std::string, int)>& f)
{
       
}
std::shared_ptr<Connection> MetaObjectFactory::ConnectConstructorAdded(TypedSlot<void(void)>* slot)
{
    return _pimpl->on_constructor_added.Connect(slot);
}
