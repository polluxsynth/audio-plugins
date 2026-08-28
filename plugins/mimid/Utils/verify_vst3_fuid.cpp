// verify_vst3_fuid.cpp
//
// Loads a VST3 plugin's shared object directly (no Steinberg SDK
// required) using DPF's own "travesty" headers -- a small pure-C
// re-declaration of the VST3 ABI that DPF itself uses to implement
// its VST3 wrapper. We call the module's exported GetPluginFactory(),
// walk the factory's classes, and print each class_id exactly as
// FUID::toString() would on Linux/macOS and on Windows (see the long
// comment in dpf_vst3_fuid.py for why those two differ).
//
// This exists to close the loop on dpf_vst3_fuid.py: that script
// computes the FUID by re-implementing DPF's arithmetic from
// DistrhoPluginInfo.h; this program instead asks the actual compiled
// plugin binary what its class id really is. If the two ever
// disagree, something about the static computation (or a DPF version
// upgrade that changed the formula) needs investigating -- trust this
// program's output over the script's in that case, since it is
// reading ground truth out of the binary itself.
//
// Build (Linux):
//   g++ -std=c++17 -I/path/to/dpf/distrho/src -o verify_vst3_fuid verify_vst3_fuid.cpp -ldl
//
// Usage:
//   ./verify_vst3_fuid /path/to/Plugin.vst3/Contents/x86_64-linux/Plugin.so
//
// (On Windows the bundle layout is Plugin.vst3\Contents\x86_64-win\Plugin.vst3
//  as a DLL; on macOS it's inside Plugin.vst3/Contents/MacOS/Plugin as a
//  bundle. Point this program directly at the loadable binary in each case;
//  dlopen()/dlsym() below would need swapping for LoadLibrary/GetProcAddress
//  on Windows.)

#include "travesty/base.h"
#include "travesty/factory.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>

// ---------------------------------------------------------------------
// hex formatting identical to dpf_vst3_fuid.py's two encodings

static void hex_linux(const uint8_t (&raw)[16], char out[33])
{
    static const char* digits = "0123456789ABCDEF";
    for (int i = 0; i < 16; ++i)
    {
        out[i * 2 + 0] = digits[(raw[i] >> 4) & 0xF];
        out[i * 2 + 1] = digits[raw[i] & 0xF];
    }
    out[32] = '\0';
}

static void hex_windows(const uint8_t (&raw)[16], char out[33])
{
    // Reinterpret the first 8 bytes as a native-endian Windows GUID
    // {DWORD Data1; WORD Data2; WORD Data3;} the same way
    // FUID::toString() does when COM_COMPATIBLE == 1, then dump the
    // remaining 8 bytes plainly.
    uint32_t data1;
    uint16_t data2, data3;
    std::memcpy(&data1, raw + 0, 4);
    std::memcpy(&data2, raw + 4, 2);
    std::memcpy(&data3, raw + 6, 2);

    char tail[17];
    static const char* digits = "0123456789ABCDEF";
    for (int i = 0; i < 8; ++i)
    {
        tail[i * 2 + 0] = digits[(raw[8 + i] >> 4) & 0xF];
        tail[i * 2 + 1] = digits[raw[8 + i] & 0xF];
    }
    tail[16] = '\0';

    std::snprintf(out, 33, "%08X%04X%04X%s", data1, data2, data3, tail);
}

// ---------------------------------------------------------------------

typedef void* (*GetPluginFactoryFunc)();
typedef bool (*ModuleEntryFunc)(void*);
typedef bool (*ModuleExitFunc)();

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::fprintf(stderr, "usage: %s /path/to/Plugin.vst3/Contents/<arch>/Plugin.so\n", argv[0]);
        return 1;
    }

    void* const handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr)
    {
        std::fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }

    // The VST3 module ABI requires the host to call the module's entry
    // point (ModuleEntry on Linux, InitDll on Windows, bundleEntry on
    // macOS -- see DistrhoPluginVST3.cpp's ENTRYFNNAME) before touching
    // the factory. DPF uses this call to lazily construct its internal
    // dummy plugin instance (sPlugin) and patch the unique id into all
    // five TUIDs; skipping it leaves sPlugin null and crashes on the
    // first getClassInfo() that touches it.
    dlerror();
    ModuleEntryFunc moduleEntry =
        reinterpret_cast<ModuleEntryFunc>(dlsym(handle, "ModuleEntry"));
    if (const char* const err = dlerror())
    {
        std::fprintf(stderr, "dlsym(ModuleEntry) failed: %s\n"
                              "(expected on Windows/macOS builds -- this program "
                              "targets Linux .so files; adapt to InitDll/bundleEntry "
                              "as noted in the file header for those platforms)\n", err);
        dlclose(handle);
        return 1;
    }
    if (!moduleEntry(handle))
    {
        std::fprintf(stderr, "ModuleEntry() returned false\n");
        dlclose(handle);
        return 1;
    }

    dlerror(); // clear
    GetPluginFactoryFunc getFactory =
        reinterpret_cast<GetPluginFactoryFunc>(dlsym(handle, "GetPluginFactory"));
    if (const char* const err = dlerror())
    {
        std::fprintf(stderr, "dlsym(GetPluginFactory) failed: %s\n", err);
        dlclose(handle);
        return 1;
    }

    void* const factoryRaw = getFactory();
    if (factoryRaw == nullptr)
    {
        std::fprintf(stderr, "GetPluginFactory() returned null\n");
        dlclose(handle);
        return 1;
    }

    // factoryRaw already points at an object whose first machine word
    // is a vtable pointer matching v3_plugin_factory_cpp's layout
    // (funknown's 3 slots, then v1's 4 slots: get_factory_info,
    // num_classes, get_class_info, create_instance). That's exactly
    // what Steinberg's IPluginFactory ABI guarantees, so we can use
    // it directly without an explicit query_interface round-trip.
    v3_plugin_factory_cpp** const factory =
        static_cast<v3_plugin_factory_cpp**>(factoryRaw);
    v3_plugin_factory_cpp* const vtable = *factory;

    const int32_t numClasses = vtable->v1.num_classes(factory);
    std::printf("num_classes: %d\n\n", numClasses);

    for (int32_t idx = 0; idx < numClasses; ++idx)
    {
        v3_class_info info;
        std::memset(&info, 0, sizeof(info));

        const v3_result res = vtable->v1.get_class_info(factory, idx, &info);
        if (res != 0 /* V3_OK */)
        {
            std::fprintf(stderr, "get_class_info(%d) failed: %d\n", idx, res);
            continue;
        }

        uint8_t raw[16];
        std::memcpy(raw, info.class_id, 16);

        char linuxHex[33], windowsHex[33];
        hex_linux(raw, linuxHex);
        hex_windows(raw, windowsHex);

        std::printf("class %d\n", idx);
        std::printf("  name      : %s\n", info.name);
        std::printf("  category  : %s\n", info.category);
        std::printf("  raw bytes : ");
        for (int i = 0; i < 16; ++i)
            std::printf("%02x", raw[i]);
        std::printf("\n");
        std::printf("  linux/mac : %s\n", linuxHex);
        std::printf("  windows   : %s\n", windowsHex);
        std::printf("\n");
    }

    dlerror();
    if (ModuleExitFunc moduleExit =
            reinterpret_cast<ModuleExitFunc>(dlsym(handle, "ModuleExit")))
        moduleExit();

    dlclose(handle);
    return 0;
}
