"""Independent PCM acceptance checks; standard library only, no audio device."""
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import struct
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]


def rms(values):
    return math.sqrt(sum(value * value for value in values) / len(values))


class AchievementSoundTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.path = ROOT / "resources/audio/achievement.wav"
        cls.raw = cls.path.read_bytes()
        cls.metadata = json.loads(cls.path.with_suffix(".json").read_text(encoding="utf-8"))
        with wave.open(str(cls.path), "rb") as source:
            cls.rate = source.getframerate()
            cls.frames = source.getnframes()
            cls.channels = source.getnchannels()
            cls.width = source.getsampwidth()
            cls.compression = source.getcomptype()
            cls.pcm = struct.unpack(f"<{cls.frames * cls.channels}h", source.readframes(cls.frames))
        cls.samples = [value / 32768 for value in cls.pcm]

    def test_pcm_duration_and_header_integrity(self):
        self.assertEqual(self.raw[:4], b"RIFF")
        self.assertEqual(self.raw[8:12], b"WAVE")
        self.assertEqual(struct.unpack_from("<I", self.raw, 4)[0] + 8, len(self.raw))
        self.assertEqual(self.raw[12:16], b"fmt ")
        self.assertEqual(struct.unpack_from("<H", self.raw, 20)[0], 1)
        self.assertEqual((self.channels, self.width, self.rate, self.compression), (1, 2, 44100, "NONE"))
        self.assertEqual(self.raw[36:40], b"data")
        self.assertEqual(struct.unpack_from("<I", self.raw, 40)[0], self.frames * 2)
        duration = self.frames / self.rate
        self.assertGreaterEqual(duration, 1.1)
        self.assertLessEqual(duration, 1.3)
        self.assertEqual(duration, self.metadata["duration_seconds"])
        self.assertEqual(len(self.raw), 44 + self.frames * 2)

    def test_peak_dc_offset_and_no_clipping(self):
        self.assertLess(max(map(abs, self.pcm)), 32767)
        self.assertLessEqual(max(map(abs, self.samples)), 0.37)
        self.assertGreater(rms(self.samples), 0.035)
        self.assertLess(rms(self.samples), 0.15)
        self.assertLess(abs(sum(self.samples) / len(self.samples)), 0.0001)

    def test_continuity_soft_attack_and_endpoints(self):
        self.assertEqual((self.pcm[0], self.pcm[-1]), (0, 0))
        self.assertLess(max(abs(b - a) for a, b in zip(self.samples, self.samples[1:])), 0.075)
        onset = self.samples[:round(self.rate * 0.01)]
        body = self.samples[round(self.rate * 0.025):round(self.rate * 0.05)]
        self.assertLess(rms(onset), rms(body) * 0.5)
        self.assertLess(rms(self.samples[-round(self.rate * 0.02):]), 0.00015)

    def test_gradual_release(self):
        size = round(self.rate * 0.1)
        tail = [rms(self.samples[-3*size:-2*size]), rms(self.samples[-2*size:-size]), rms(self.samples[-size:])]
        self.assertGreater(tail[0], tail[1])
        self.assertGreater(tail[1], tail[2])
        self.assertLess(tail[2], tail[0] * 0.25)

    def test_body_is_extended_instead_of_silent_padding(self):
        total = sum(value * value for value in self.samples)
        energy = 0
        t95 = t99 = None
        for index, value in enumerate(self.samples):
            energy += value * value
            if t95 is None and energy >= total * 0.95:
                t95 = index / self.rate
            if t99 is None and energy >= total * 0.99:
                t99 = index / self.rate
        self.assertGreater(t95, 0.5)
        self.assertGreater(t99, 0.65)
        size = round(self.rate * 0.02)
        windows = [rms(self.samples[i:i + size]) for i in range(0, len(self.samples), size)]
        active = [i for i, amplitude in enumerate(windows) if amplitude >= max(windows) * 0.01]
        # Relative RMS proxy only; not a physical audibility threshold.
        self.assertGreaterEqual((active[-1] + 1) * 0.02, self.frames / self.rate * 0.9)
        last_nonzero = max(i for i, value in enumerate(self.pcm) if value)
        self.assertLess((self.frames - last_nonzero - 1) / self.rate, 0.025)

    def test_three_fundamentals_remain_present(self):
        amplitudes = []
        for hz in (523.251, 659.255, 880.0):
            real = sum(value * math.cos(2 * math.pi * hz * i / self.rate) for i, value in enumerate(self.samples))
            imaginary = sum(value * math.sin(2 * math.pi * hz * i / self.rate) for i, value in enumerate(self.samples))
            amplitudes.append(2 * math.hypot(real, imaginary) / self.frames)
        self.assertGreater(min(amplitudes), 0.01)
        self.assertGreater(min(amplitudes) / max(amplitudes), 0.35)

    def test_reproducible_bytes_metadata_and_hash(self):
        spec = importlib.util.spec_from_file_location("glossata_chime", ROOT / "scripts/generate_achievement_sound.py")
        generator = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(generator)
        data, metadata = generator.synthesize()
        metadata["sha256"] = hashlib.sha256(data).hexdigest()
        self.assertEqual(data, self.raw)
        self.assertEqual(metadata, self.metadata)
        self.assertFalse(metadata["external_samples"])
        self.assertEqual(metadata["licensing_status"], "pending-maintainer-decision")


if __name__ == "__main__":
    unittest.main()
