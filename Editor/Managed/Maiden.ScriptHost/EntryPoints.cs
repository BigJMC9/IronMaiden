using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using Maiden;

namespace Maiden.ScriptHost;

internal sealed class GameLoadContext : AssemblyLoadContext
{
    private readonly AssemblyDependencyResolver _resolver;

    public GameLoadContext(string assemblyPath) : base(isCollectible: true)
        => _resolver = new AssemblyDependencyResolver(assemblyPath);

    protected override Assembly? Load(AssemblyName name)
    {
        // The game and the host must share the same Behaviour type.
        if (name.Name == typeof(Behaviour).Assembly.GetName().Name) return typeof(Behaviour).Assembly;
        string? path = _resolver.ResolveAssemblyToPath(name);
        return path is null ? null : LoadFromAssemblyPath(path);
    }
}

public static unsafe class EntryPoints
{
    private static GameLoadContext? _context;
    private static readonly Dictionary<string, Type> Types = new(StringComparer.Ordinal);
    private static readonly Dictionary<uint, Behaviour> Instances = new();

    [UnmanagedCallersOnly]
    public static int Start(byte* assemblyPath, NativeCallbacks* callbacks)
    {
        try
        {
            StopCore();
            if (callbacks is null) return 0;
            NativeApi.Initialize(*callbacks);
            string path = Marshal.PtrToStringUTF8((nint)assemblyPath) ?? string.Empty;
            if (!File.Exists(path)) throw new FileNotFoundException("Game scripts assembly is missing", path);
            _context = new GameLoadContext(path);
            // Loading from a stream allows the project to rebuild while the editor stays open.
            using var stream = File.OpenRead(path);
            Assembly assembly = _context.LoadFromStream(stream);
            foreach (Type type in assembly.GetTypes().Where(t => !t.IsAbstract && typeof(Behaviour).IsAssignableFrom(t)))
            {
                if (type.GetConstructor(Type.EmptyTypes) is null) continue;
                if (type.FullName is not null) Types[type.FullName] = type;
                if (!Types.ContainsKey(type.Name)) Types[type.Name] = type;
            }
            Log.Info($"Loaded {Types.Count} C# behaviour names.");
            return 1;
        }
        catch (Exception e)
        {
            Log.Error($"C# load failed: {e}");
            StopCore();
            return 0;
        }
    }

    [UnmanagedCallersOnly]
    public static int Create(uint entity, byte* className)
    {
        try
        {
            string name = Marshal.PtrToStringUTF8((nint)className) ?? string.Empty;
            if (!Types.TryGetValue(name, out Type? type))
            {
                Log.Error($"C# behaviour '{name}' was not found.");
                return 0;
            }
            var instance = (Behaviour)Activator.CreateInstance(type)!;
            instance.Entity = new Entity(entity);
            Instances[entity] = instance;
            instance.OnCreate();
            return 1;
        }
        catch (Exception e)
        {
            Log.Error($"C# OnCreate failed: {e}");
            Instances.Remove(entity);
            return 0;
        }
    }

    [UnmanagedCallersOnly]
    public static void Update(float deltaTime)
    {
        foreach (var (entity, instance) in Instances.ToArray())
        {
            try { instance.OnUpdate(deltaTime); }
            catch (Exception e) { Log.Error($"C# OnUpdate failed on entity {entity}: {e}"); }
        }
    }

    [UnmanagedCallersOnly]
    public static void Stop() => StopCore();

    private static void StopCore()
    {
        foreach (var instance in Instances.Values)
        {
            try { instance.OnDestroy(); }
            catch (Exception e) { Log.Error($"C# OnDestroy failed: {e}"); }
        }
        Instances.Clear();
        Types.Clear();
        GameLoadContext? context = _context;
        _context = null;
        context?.Unload();
    }
}
