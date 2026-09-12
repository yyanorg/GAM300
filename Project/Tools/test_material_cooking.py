"""Check that a clean desktop cook preserves authored material settings."""

import argparse
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile


def read_material(path):
    data = path.read_bytes()
    offset = 0

    def unpack(fmt):
        nonlocal offset
        values = struct.unpack_from("<" + fmt, data, offset)
        offset += struct.calcsize("<" + fmt)
        return values[0] if len(values) == 1 else values

    def string():
        nonlocal offset
        length = unpack("Q")
        value = data[offset:offset + length].decode("utf-8")
        offset += length
        return value

    # The desktop .mat format stores size_t lengths, 17 property floats,
    # texture type/path pairs, and optional tiling/offset values.
    name = string()
    properties = unpack("17f")
    textures = []
    for _ in range(unpack("Q")):
        texture_type = unpack("i")
        textures.append((texture_type, string()))
    return name, properties, sorted(textures), data[offset:]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cooker", type=Path, required=True)
    parser.add_argument("--resources", type=Path,
                        default=Path(__file__).resolve().parents[1] / "Resources")
    args = parser.parse_args()
    cooker = args.cooker.resolve()
    source = args.resources.resolve()

    with tempfile.TemporaryDirectory(prefix="kusane-material-test-") as temp:
        root = Path(temp)
        resources = root / "Resources"
        working = root / "Build" / "Release"
        working.mkdir(parents=True)

        def copy(relative):
            target = resources / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source / relative, target)

        # These shipped materials lost their texture maps and colours in a
        # clean cook. Create Models first to exercise discovery before all
        # materials have been registered, and pass an absolute resource root.
        models = ("WoodenBarrier", "WeaponHolder", "WallTop")
        for name in models:
            for extension in (".fbx", ".fbx.meta"):
                copy(Path("Models/Environment") / (name + extension))

        expected = {}
        for name in models:
            materials = list((source / "Materials").glob(name + "_*.mat"))
            assert materials, f"Missing fixture materials for {name}"
            for material in materials:
                relative = material.relative_to(source)
                expected[relative] = read_material(material)
                copy(relative)
                copy(Path(str(relative) + ".meta"))
                for _, texture in expected[relative][2]:
                    relative_texture = Path(texture[texture.index("Resources/") + 10:])
                    copy(relative_texture)
                    copy(Path(str(relative_texture) + ".meta"))

        env = os.environ.copy()
        if os.name != "nt":
            env["LD_LIBRARY_PATH"] = str(cooker.parent) + os.pathsep + env.get("LD_LIBRARY_PATH", "")
        result = subprocess.run([str(cooker), "--resources", str(resources)],
                                cwd=working, env=env, capture_output=True,
                                text=True, errors="replace")
        print(result.stdout)
        if result.returncode:
            raise RuntimeError(f"AssetCooker exited {result.returncode}: {result.stderr}")
        for relative, original in expected.items():
            assert read_material(resources / relative) == original, \
                f"Cooking changed the authored material: {relative}"
        assert len(list(resources.rglob("*.mesh"))) == len(models)
        assert list(resources.rglob("*.dds")), "No textures were cooked"
        print(f"Preserved {len(expected)} authored materials through a clean cook.")


if __name__ == "__main__":
    main()
