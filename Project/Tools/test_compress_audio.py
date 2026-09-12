"""Verify lossless audio staging and preservation of asset identities."""

import json
from pathlib import Path
import tempfile
import unittest

import numpy as np
import soundfile as sf

from compress_audio import compress_audio


class AudioCompressionTests(unittest.TestCase):
    def test_samples_and_guids_survive_compression(self):
        with tempfile.TemporaryDirectory() as directory:
            resources = Path(directory) / 'Resources'
            resources.mkdir()
            samples = (np.sin(np.arange(144000) * 0.1) * (2**29)).astype('int32')
            originals = {}
            for subtype in ('PCM_16', 'PCM_24'):
                path = resources / (subtype + '.wav')
                sf.write(path, samples, 48000, subtype=subtype)
                originals[subtype] = sf.read(path, dtype='int32')[0]
                Path(str(path) + '.meta').write_text(json.dumps({'AssetMetaData': {
                    'guid': subtype, 'source': 'Resources/' + path.name,
                    'compiled': '../../Resources/' + path.name
                }}))
            compress_audio(resources)
            for subtype, original in originals.items():
                path = resources / (subtype + '.flac')
                decoded, rate = sf.read(path, dtype='int32')
                self.assertEqual(rate, 48000)
                self.assertTrue(np.array_equal(original, decoded))
                self.assertFalse(path.with_suffix('.wav').exists())
                meta = json.loads((resources / (subtype + '.wav.meta')).read_text())['AssetMetaData']
                self.assertEqual(meta['guid'], subtype)
                self.assertEqual(meta['compiled'], 'Resources/' + path.name)

    def test_small_and_float_effects_stay_unchanged(self):
        with tempfile.TemporaryDirectory() as directory:
            resources = Path(directory)
            sf.write(resources / 'small.wav', np.zeros(100), 48000, subtype='PCM_16')
            sf.write(resources / 'float.wav', np.linspace(-1.2, 1.2, 144000), 48000, subtype='FLOAT')
            originals = {path.name: path.read_bytes() for path in resources.iterdir()}
            compress_audio(resources)
            self.assertEqual({path.name: path.read_bytes() for path in resources.iterdir()}, originals)


if __name__ == '__main__':
    unittest.main()
