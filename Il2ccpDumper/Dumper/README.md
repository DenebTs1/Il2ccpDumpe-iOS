# Il2ccpDumper (C++ runtime port)

A C++ port of [Il2CppDumper](https://github.com/Perfare/Il2CppDumper)
designed to run **inside** an iOS app rather than as a desktop tool.

The port targets arm64 Mach-O binaries (FEEDFACF) and modern Unity (Il2Cpp v24+
through v31), so most of the C# project's executable-format support (PE, ELF,
NSO, WebAssembly) and the DummyDLL emitter are intentionally omitted.

---

## Layout

```
Dumper/
├── Il2Dumper.h / .mm            Public facade: Il2Dumper::m_Run(...)
├── Il2DumperImGui.h / .mm       Optional ImGui button hook
├── Config.h                     SDumpConfig + SDumpSources structs
├── Core/
│   ├── Types.h                  E_Status, E_Format, E_MetadataUsage, constants
│   └── Version.h                SVersion helper
├── IO/
│   ├── IDataSource.h            Abstract byte buffer
│   ├── CMemorySource.h          Borrows live process bytes
│   ├── CFileSource.h/.cpp       Reads a whole file into memory
│   └── CBinaryStream.h          Little-endian random-access reader
├── ExecutableFormats/
│   ├── SMachoTypes.h            POD section/slice/search-section structs
│   ├── CMachoFat.h/.cpp         Thin fat-archive reader + arm64 slice picker
│   └── CMachO64.h/.cpp          Mach-O 64 parser, VA<->offset mapping
├── Il2Cpp/
│   ├── MetadataTypes.h/.cpp     Per-record metadata POD layouts
│   ├── CMetadata.h/.cpp         global-metadata.dat reader
│   ├── Il2CppTypes.h/.cpp       Code/Metadata registration + type structs
│   └── CIl2Cpp.h/.cpp           Binary parser + PlusSearch driver
├── Outputs/
│   ├── Il2CppConstants.h        ECMA-335 flag constants
│   ├── CDecompiler.h/.cpp       dump.cs writer
│   ├── CStructGenerator.h/.cpp  il2cpp.h writer (minimal)
│   ├── CScriptJson.h/.cpp       script.json writer
│   └── CStringLiteralJson.h/.cpp stringliteral.json writer
└── Utils/
    ├── CSectionHelper.h/.cpp    Code/MetadataRegistration locator
    └── CIl2CppExecutor.h/.cpp   Joins metadata + binary, name resolution
```

---

## Naming conventions

Every identifier follows a fixed prefix scheme. The codebase is consistent end
to end -- if you're adding code, match the pattern.

| Kind              | Prefix       | Example                       |
|-------------------|--------------|-------------------------------|
| `bool` variable   | `b`          | `bIsDumped`, `b_FieldOffsetsArePointers_` |
| Enum declaration  | `E_`         | `enum class E_Status { ... }` |
| Enum values       | `E_`         | `E_Status::E_Ok`              |
| Raw pointer       | `p_`         | `p_Mach`, `p_Bytes`           |
| Struct (POD)      | `S`          | `SDumpConfig`, `SMachoSection64` |
| Class             | `C`          | `CMachO64`, `CMetadata`       |
| Interface         | `I`          | `IDataSource`                 |
| Member variable   | `m_`         | `m_Version`, `m_Offset`       |
| Function (member) | `m_` (or `b_` if returning bool) | `m_Read()`, `b_Init()` |
| Free function     | `s_`         | `s_BE32(...)`                 |
| Global constant   | `k_`         | `k_MetadataSanity`            |

Brace style is Allman (`if (...) \n {`) -- the project formatter enforces it.

---

## Public API

Two layers:

**C++ direct**
```cpp
#include "Dumper/Il2Dumper.h"

Il2Dumper::SDumpSources m_Src;
m_Src.p_Il2cppMemory   = ...;  m_Src.m_Il2cppSize   = ...;
m_Src.p_MetadataMemory = ...;  m_Src.m_MetadataSize = ...;
m_Src.m_ImageBase = ...;       // VM base of the il2cpp slice
m_Src.bIsDumped   = true;      // when reading the running process

Il2Dumper::SDumpConfig m_Cfg;
Il2Dumper::E_Status m_St = Il2Dumper::m_Run(m_Src, m_Cfg);
```

**ObjC bridge** (the existing `Il2ccpDumper` class)
```objc
NSError* err = nil;
[Il2ccpDumper dumpFromFilesIl2cpp:@"/path/to/libil2cpp"
                         metadata:@"/path/to/global-metadata.dat"
                        outputDir:nil           // -> Documents/Il2cppDump/
                            error:&err];
```

By default both paths are supported (file or memory). When `m_ImageBase != 0`
the dumper treats the il2cpp source as a memory-dumped image (`bIsDumped`).

---

## Outputs

All written under `<outputDir>/` (defaults to `Documents/Il2cppDump/`):

| File                  | Content                                                |
|-----------------------|--------------------------------------------------------|
| `dump.cs`             | Per-namespace breakdown with classes, fields, methods, offsets, tokens. Informational, not valid C#. |
| `il2cpp.h`            | C header with one `struct` per managed type (minimal -- single-field shells), for IDA/Ghidra script consumption. |
| `script.json`         | `{ Address, Name, Signature }` triples for every resolved method pointer. |
| `stringliteral.json`  | `[{ index, value }]` for every string literal in the metadata. |

---

## ImGui menu hook

The dumper ships an optional ImGui entry. Register a source provider once on
startup, then call the draw function inside any window:

```cpp
#include "Dumper/Il2DumperImGui.h"

static void s_MyProvider(Il2Dumper::SDumpSources& m_Out)
{
    m_Out.p_Il2cppMemory   = MyGetIl2cppBase();
    m_Out.m_Il2cppSize     = MyGetIl2cppSize();
    m_Out.p_MetadataMemory = MyGetMetadataBase();
    m_Out.m_MetadataSize   = MyGetMetadataSize();
    m_Out.m_ImageBase      = (uint64_t)MyGetIl2cppBase();
    m_Out.bIsDumped        = true;
}

// once at startup:
Il2Dumper::m_SetImGuiSourceProvider(&s_MyProvider);

// every frame, inside any ImGui window:
Il2Dumper::m_DrawImGuiMenu();
```

The button runs the dump on a background queue and shows the resulting path
once it completes.

---

## Status / known scope

This is a runtime port, not a 1:1 port. The big-ticket items still on the
table:

- **Custom attribute output**: skipped. The C# `Il2CppDecompiler.GetCustomAttribute`
  walks attribute blobs to reconstruct `[Xxx(...)]` syntax. A future pass can
  port `CustomAttributeDataReader.cs`.
- **Generic instantiation rendering**: `m_GetTypeName` returns `<generic>` for
  `IL2CPP_TYPE_GENERICINST`. The C# version walks `Il2CppGenericClass` +
  `Il2CppGenericInst` to produce `Foo<Bar, Baz>` -- straightforward but
  verbose; not yet ported.
- **RGCTX dump**: the C# emits per-method RGCTX details into `script.json`.
  We emit method addresses only.
- **Version coverage**: the metadata reader handles v16-v31. The Il2Cpp
  parser handles v24.2 through v31 paths fully; v22-v24.1 are wired up but
  the C# parser had extra branches for pre-24.2 method-pointer layout that
  are exercised less frequently. If you hit those, follow the pattern in
  `b_Init()` and `m_AutoPlusInit()` in `CIl2Cpp.cpp`.

The reader-per-struct pattern (every `Sxxx` has a `Read(stream, version)`
method that gates fields by version) mirrors the C# `[Version(Min,Max)]`
attribute model. It looked tempting to make these per-version C++ templates
but the version matrix (16, 19, 20, 21, 22, 23, 24, 24.1...24.5, 27, 27.1,
27.2, 29, 29.1, 31) makes that unmaintainable. The runtime cost is one
read of the metadata header per dump, which is negligible.

---

## Adding to the Xcode target

The folder is plain C++ + ObjC++. Add the `Dumper/` folder reference to the
`Il2ccpDumper` Xcode target (Build Phases -> Compile Sources for all `.cpp`
and `.mm` files; headers are pulled in by include paths). The code expects
C++17.

Make sure `Il2DumperImGui.mm` is only compiled when ImGui is in the link
graph; otherwise drop that file from the target.
