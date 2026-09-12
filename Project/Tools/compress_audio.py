"""Compress packaged PCM music and longer effects without changing their samples."""

import argparse
import json

from stage_resources import compiled_path
from pathlib import Path


def compress_audio(resources):
    import numpy as np
    import soundfile as sf

    converted = 0
    saved = 0
    for source in sorted(resources.rglob('*')):
        if source.suffix.lower() != '.wav' or not source.is_file():
            continue
        # Small effects and floating-point recordings retain their original data.
        if source.stat().st_size < 256 * 1024:
            continue
        info = sf.info(source)
        if info.subtype not in ('PCM_16', 'PCM_24'):
            continue
        meta_path = Path(str(source) + '.meta')
        metadata = json.loads(meta_path.read_text(encoding='utf-8'))
        meta = metadata['AssetMetaData']
        if not isinstance(meta.get('guid'), str) or not isinstance(meta.get('compiled'), str):
            raise ValueError(f'Invalid audio metadata: {meta_path}')
        if compiled_path(resources, meta['compiled']) != source.resolve():
            continue
        target = source.with_suffix('.flac')
        if target.exists():
            raise ValueError(f'Audio output already exists: {target}')
        samples, rate = sf.read(source, dtype='int32', always_2d=True)
        sf.write(target, samples, rate, subtype=info.subtype, compression_level=1.0)
        decoded, decoded_rate = sf.read(target, dtype='int32', always_2d=True)
        if decoded_rate != rate or not np.array_equal(samples, decoded):
            target.unlink()
            raise ValueError(f'Audio verification failed: {source}')
        difference = source.stat().st_size - target.stat().st_size
        if difference <= 0:
            target.unlink()
            continue
        # Keep the original asset name/GUID. Only its packaged resource changes.
        relative = target.relative_to(resources).as_posix()
        meta['compiled'] = 'Resources/' + relative
        meta_path.write_text(json.dumps(metadata, indent=4) + '\n', encoding='utf-8')
        source.unlink()
        saved += difference
        converted += 1
    print(f'Losslessly compressed {converted} audio files; saved {saved / 1048576:.1f} MiB.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-directory', type=Path, required=True,
                        help='Staged game directory containing Kusane and Resources')
    args = parser.parse_args()
    resources = args.game_directory / 'Resources'
    if not resources.is_dir() or not any((args.game_directory / name).is_file()
                                         for name in ('Kusane.exe', 'Kusane')):
        parser.error('expected a staged game directory containing Kusane and Resources')
    compress_audio(resources.resolve())
