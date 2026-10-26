"""Deterministic Glossata chime: additive synthesis, no samples or external input.

The generated audio has a pending licensing decision; see resources/audio/README.md.
"""
import argparse
import hashlib
import io
import json
import math
from pathlib import Path
import struct
import wave

ROOT = Path(__file__).resolve().parents[1]
DESTINATION = ROOT / "resources/audio/achievement.wav"


def synthesize():
    rate, duration = 44100, 0.85
    notes = ((523.251, 0.0, 1.0), (659.255, 0.095, 0.72), (880.0, 0.19, 0.6))
    samples = []
    for index in range(round(rate * duration)):
        t = index / rate
        sample = 0.0
        for frequency, start, gain in notes:
            age = t - start
            if age < 0:
                continue
            attack = math.sin(min(1.0, age / 0.018) * math.pi / 2) ** 2
            tone = (math.sin(2 * math.pi * frequency * age)
                    + 0.18 * math.sin(2 * math.pi * frequency * 2.01 * age)
                    + 0.05 * math.sin(2 * math.pi * frequency * 3 * age))
            sample += gain * attack * math.exp(-6.5 * age) * tone
        release = math.sin(min(1.0, (duration - t) / 0.16) * math.pi / 2) ** 2
        samples.append(sample * release)
    gain = 0.36 / max(abs(x) for x in samples)
    pcm = [round(sample * gain * 32767) for sample in samples]
    pcm[0] = pcm[-1] = 0
    stream = io.BytesIO()
    with wave.open(stream, "wb") as output:
        output.setparams((1, 2, rate, len(pcm), "NONE", "not compressed"))
        output.writeframes(struct.pack(f"<{len(pcm)}h", *pcm))
    return stream.getvalue(), {
        "sample_rate": rate, "channels": 1, "bits_per_sample": 16,
        "duration_seconds": duration, "peak_dbfs": round(20 * math.log10(max(abs(x) for x in pcm) / 32768), 3),
        "rms_dbfs": round(20 * math.log10(math.sqrt(sum(x*x for x in pcm) / len(pcm)) / 32768), 3),
        "first_sample": pcm[0], "last_sample": pcm[-1],
        "external_samples": False, "licensing_status": "pending-maintainer-decision",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    data, metadata = synthesize()
    metadata["sha256"] = hashlib.sha256(data).hexdigest()
    if args.check:
        if not DESTINATION.exists() or DESTINATION.read_bytes() != data:
            raise SystemExit("The stored WAV does not match the documented synthesis.")
        if json.loads(DESTINATION.with_suffix(".json").read_text(encoding="utf-8")) != metadata:
            raise SystemExit("The stored audio metadata does not match the synthesis.")
    else:
        DESTINATION.parent.mkdir(parents=True, exist_ok=True)
        DESTINATION.write_bytes(data)
        DESTINATION.with_suffix(".json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(json.dumps(metadata, indent=2))


if __name__ == "__main__":
    main()
