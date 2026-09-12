"""Checks for preserving runtime inputs and rejecting incomplete cooked builds."""

import json
from pathlib import Path
import tempfile
import unittest

from stage_resources import compiled_path, stage_resources


class ResourceStagingTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.resources = self.root / 'Resources'
        self.resources.mkdir()
        self.destination = self.root / 'package' / 'Resources'

    def put(self, path, data=b'asset'):
        target = self.resources / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        return target

    def texture(self, compiled='..\\..\\Resources\\Textures\\test.dds'):
        self.put('Textures/test.png')
        self.put('Textures/test.png.meta', json.dumps({'AssetMetaData': {
            'guid': '0000000000000001-0000000000000002', 'compiled': compiled
        }}).encode())

    def test_preserves_runtime_inputs_and_metadata(self):
        self.texture()
        self.put('Textures/test.dds', b'cooked texture')
        self.put('Animations/run.fbx', b'runtime animation')
        self.put('Scripts/player.lua', b'return {}')
        self.put('Licenses/font.txt', b'font license')
        self.put('Scenes/test.scene.temp')
        stage_resources(self.resources, self.destination)
        self.assertFalse((self.destination / 'Textures/test.png').exists())
        self.assertFalse((self.destination / 'Scenes/test.scene.temp').exists())
        for name in ('Textures/test.png.meta', 'Textures/test.dds',
                     'Animations/run.fbx', 'Scripts/player.lua', 'Licenses/font.txt'):
            self.assertEqual((self.destination / name).read_bytes(),
                             (self.resources / name).read_bytes())

    def test_missing_cooked_file_fails_before_copying(self):
        self.put('Configs/input_config.json')
        self.texture()
        with self.assertRaisesRegex(ValueError, 'missing or empty'):
            stage_resources(self.resources, self.destination)
        self.assertFalse(self.destination.exists())

    def test_rejects_paths_outside_resources(self):
        for path in ('../../secret.dds', '../../Resources/../secret.dds'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                compiled_path(self.resources, path)

    def test_rejects_source_overwrite_and_stale_destination(self):
        with self.assertRaises(ValueError):
            stage_resources(self.resources, self.resources)
        self.destination.mkdir(parents=True)
        (self.destination / 'stale.png').write_bytes(b'old')
        with self.assertRaisesRegex(ValueError, 'empty'):
            stage_resources(self.resources, self.destination)


if __name__ == '__main__':
    unittest.main()
