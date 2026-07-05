# On-Device Model Catalog

This directory hosts the manifests for every on-device model shipped by SS-SP. Each model has a `PROVENANCE.md` describing:

- `model_id`
- Purpose (STT / TTS / LLM assist / VAD / etc.)
- Underlying architecture
- Training-data provenance (public datasets, licenses, dates)
- Sizes (params, quantization, on-flash bytes)
- Hardware targets (Lite ESP32-S3, Alpha ESP32-P4, etc.)
- License(s)
- Signature key used to sign the model artifact
- Known limitations and safety notes

## Non-negotiable rules

- **No model with unclear training-data provenance ships.** Compliance with the Reticulum License (no AI training on RNS-touching data) also requires we do not knowingly ship models trained on such data.
- Every model artifact is signed with `RelKey_A` (see `05_SECURITY_MODEL.md` §6.7).
- Models are delivered on a **separate partition** (`fs_models`) to avoid bloating app slots and enable independent updates.

## Initial catalog (planned)

| model_id | Purpose | Params | Quant | Bytes | License | Target |
|---|---|---|---|---|---|---|
| `whisper-tiny.en-int8` | STT | 39 M | int8 | ~40 MB | MIT | Alpha, Omega |
| `vosk-small-en-us` | STT (fallback) | ~10 M | int8 | ~30 MB | Apache-2.0 | Alpha |
| `piper-en-us-lessac-low` | TTS | ~5 M | int8 | ~15 MB | MIT | Alpha, Omega |
| `piper-es-mx-claude-low` | TTS (Spanish) | ~5 M | int8 | ~15 MB | MIT | Alpha, Omega |
| `llama-3.2-1b-q4_k_m` | LLM assist | 1.2 B | q4_K_M | ~800 MB | Llama-3.2 License | Omega only |

*Lite (ESP32-S3, 8 MB PSRAM) is not a target for on-device LLM. STT/TTS on Lite is optional and heavily quantized.*
