"""Build the editor's PNG textures from the bundled Godot SVG icons."""

from pathlib import Path
import re
import xml.etree.ElementTree as ET

from PIL import Image, ImageColor, ImageDraw


ROOT = Path(__file__).resolve().parents[1] / "resources" / "icons"
SOURCE = ROOT / "vendors" / "godot"
DESTINATION = ROOT / "godot-raster"
ICONS = {
    "PlayButton": "Play",
    "PauseButton": "Pause",
    "StopButton": "Stop",
    "Folder": "Folder",
    "File": "File",
    "Scene": "PackedScene",
    "Script": "Script",
    "Shader": "Shader",
    "Camera": "Camera3D",
}
TOKENS = re.compile(r"[MmLlHhVvCcQqAaZz]|[-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?")
ARGUMENTS = {"M": 2, "L": 2, "H": 1, "V": 1, "C": 6, "Q": 4, "A": 7, "Z": 0}


def contours(data):
    tokens = TOKENS.findall(data)
    index = 0
    command = None
    point = (0.0, 0.0)
    start = point
    contour = []
    while index < len(tokens):
        if tokens[index].isalpha():
            command = tokens[index]
            index += 1
        if command is None:
            raise ValueError("SVG path has no command")
        kind = command.upper()
        if kind == "Z":
            if contour:
                contour.append(start)
                yield contour
                contour = []
            point = start
            command = None
            continue
        count = ARGUMENTS[kind]
        values = [float(value) for value in tokens[index:index + count]]
        if len(values) != count:
            raise ValueError("Incomplete SVG path")
        index += count
        relative = command.islower()

        def absolute(x, y):
            return (point[0] + x, point[1] + y) if relative else (x, y)

        if kind == "M":
            if contour:
                yield contour
            point = absolute(*values)
            start = point
            contour = [point]
            command = "l" if relative else "L"
        elif kind == "L":
            point = absolute(*values)
            contour.append(point)
        elif kind == "H":
            point = (point[0] + values[0], point[1]) if relative else (values[0], point[1])
            contour.append(point)
        elif kind == "V":
            point = (point[0], point[1] + values[0]) if relative else (point[0], values[0])
            contour.append(point)
        elif kind in ("C", "Q"):
            origin = point
            if kind == "C":
                control1, control2, end = absolute(*values[:2]), absolute(*values[2:4]), absolute(*values[4:])
                for step in range(1, 13):
                    t = step / 12
                    u = 1 - t
                    contour.append((u**3 * origin[0] + 3*u*u*t*control1[0] + 3*u*t*t*control2[0] + t**3*end[0],
                                    u**3 * origin[1] + 3*u*u*t*control1[1] + 3*u*t*t*control2[1] + t**3*end[1]))
            else:
                control, end = absolute(*values[:2]), absolute(*values[2:])
                for step in range(1, 13):
                    t = step / 12
                    u = 1 - t
                    contour.append((u*u*origin[0] + 2*u*t*control[0] + t*t*end[0],
                                    u*u*origin[1] + 2*u*t*control[1] + t*t*end[1]))
            point = end
        elif kind == "A":
            # The bundled small icons use arcs for tiny rounded corners.
            point = absolute(values[5], values[6])
            contour.append(point)
    if contour:
        yield contour


def rasterize(source, destination):
    tree = ET.parse(source)
    root = tree.getroot()
    scale = 8
    width = int(root.attrib.get("width", 16))
    height = int(root.attrib.get("height", 16))
    image = Image.new("RGBA", (width * scale, height * scale), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    for element in root:
        name = element.tag.rsplit("}", 1)[-1]
        fill = element.attrib.get("fill", "#000000")
        opacity = float(element.attrib.get("fill-opacity", "1"))
        color = (*ImageColor.getrgb(fill), round(255 * opacity)) if fill != "none" else None
        if name == "path" and color:
            for contour in contours(element.attrib["d"]):
                if len(contour) > 2:
                    draw.polygon([(round(x * scale), round(y * scale)) for x, y in contour], fill=color)
        elif name == "rect" and color:
            x = float(element.attrib.get("x", 0)) * scale
            y = float(element.attrib.get("y", 0)) * scale
            w = float(element.attrib["width"]) * scale
            h = float(element.attrib["height"]) * scale
            radius = float(element.attrib.get("rx", 0)) * scale
            draw.rounded_rectangle((x, y, x + w, y + h), radius=radius, fill=color)
        elif name == "circle":
            x, y, radius = (float(element.attrib[key]) * scale for key in ("cx", "cy", "r"))
            box = (x - radius, y - radius, x + radius, y + radius)
            if color:
                draw.ellipse(box, fill=color)
            stroke = element.attrib.get("stroke")
            if stroke:
                draw.ellipse(box, outline=ImageColor.getrgb(stroke),
                             width=max(1, round(float(element.attrib.get("stroke-width", 1)) * scale)))
    destination.parent.mkdir(parents=True, exist_ok=True)
    image.resize((64, 64), Image.Resampling.LANCZOS).save(destination)


if __name__ == "__main__":
    for output, source in ICONS.items():
        rasterize(SOURCE / f"{source}.svg", DESTINATION / f"{output}.png")
