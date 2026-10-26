# C# scripts and shader assets

In the project browser, right click inside `Assets` and choose **Create → Shader**. This creates a `.frag` file and a matching `.vert` file based on the standard shader. Edit the GLSL source, select the fragment file, and press **Compile Shader**. Compiled SPIR-V is written to the project's `Library/Shaders` folder. A material's Shader field can select the fragment asset. Material texture fields come from `layout(set = 1, binding = N) uniform sampler2D` declarations in the fragment shader.

Choose **Create → C# Script** to create a `.cs` behaviour. Add a **C# Script** component to an entity, select the script asset, and press **Play**. The editor compiles project scripts with the .NET 8 SDK and calls `OnCreate`, `OnUpdate`, and `OnDestroy`. **Stop** restores the editor scene. The .NET 8 SDK is needed to build project scripts; the .NET 8 runtime executes them.

```csharp
using Maiden;

public class Move : Behaviour
{
    public override void OnUpdate(float deltaTime)
    {
        var position = Entity.Position;
        position.X += deltaTime;
        Entity.Position = position;
    }
}
```

The class defaults to the file name. Use the component's **Class** field for a namespace qualified class name.
