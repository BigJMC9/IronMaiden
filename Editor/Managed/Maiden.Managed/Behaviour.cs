using System;
using System.Runtime.InteropServices;

namespace Maiden;

[StructLayout(LayoutKind.Sequential)]
public struct Vector3
{
    public float X;
    public float Y;
    public float Z;

    public Vector3(float x, float y, float z) => (X, Y, Z) = (x, y, z);
}

public sealed class Entity
{
    public Entity(uint id) => Id = id;
    public uint Id { get; }

    public Vector3 Position
    {
        get => NativeApi.GetPosition(Id);
        set => NativeApi.SetPosition(Id, value);
    }
}

public abstract class Behaviour
{
    public Entity Entity { get; set; } = new Entity(0);
    public virtual void OnCreate() { }
    public virtual void OnUpdate(float deltaTime) { }
    public virtual void OnDestroy() { }
}

public static class Log
{
    public static void Info(string message) => NativeApi.Log(message, 1);
    public static void Warn(string message) => NativeApi.Log(message, 2);
    public static void Error(string message) => NativeApi.Log(message, 3);
}

[StructLayout(LayoutKind.Sequential)]
public unsafe struct NativeCallbacks
{
    public delegate* unmanaged[Cdecl]<uint, float*, float*, float*, int> GetPosition;
    public delegate* unmanaged[Cdecl]<uint, float, float, float, int> SetPosition;
    public delegate* unmanaged[Cdecl]<int, byte*, void> Log;
}

public static unsafe class NativeApi
{
    private static NativeCallbacks _callbacks;

    public static void Initialize(NativeCallbacks callbacks) => _callbacks = callbacks;

    internal static Vector3 GetPosition(uint id)
    {
        float x = 0, y = 0, z = 0;
        if (_callbacks.GetPosition == null || _callbacks.GetPosition(id, &x, &y, &z) == 0)
            throw new InvalidOperationException($"Entity {id} has no transform.");
        return new Vector3(x, y, z);
    }

    internal static void SetPosition(uint id, Vector3 position)
    {
        if (_callbacks.SetPosition == null ||
            _callbacks.SetPosition(id, position.X, position.Y, position.Z) == 0)
            throw new InvalidOperationException($"Entity {id} has no transform.");
    }

    public static void Log(string message, int level)
    {
        if (_callbacks.Log == null) return;
        byte[] bytes = System.Text.Encoding.UTF8.GetBytes(message + '\0');
        fixed (byte* data = bytes) _callbacks.Log(level, data);
    }
}
