# Contagion USB Editor 1.2

Open `Contagion-USB-Editor.html` in Chrome or Edge. Connect the FM‑1 with a USB‑C data cable. Click **Connect USB MIDI**, allow MIDI and SysEx access, and select the FM‑1 MIDI input and output if they were not selected automatically.

If the browser blocks local MIDI access, start a loopback server in this directory.

```sh
python -m http.server 8765 --bind 127.0.0.1
```

Then open [the local editor](http://127.0.0.1:8765/Contagion-USB-Editor.html). On Windows, use `py` in place of `python` if needed.

- **Recall factory sound** sends the chosen program to the FM‑1. **Load into editor** opens its parameters and modulation routes.
- **Send supported knobs live** sends parameter CCs as you change values. Patch-only settings take effect after writing and recalling a USER slot.
- **Read slot** copies a stored patch into the editor. **Write slot** saves it and verifies it by reading it back. On the FM‑1, open **SCL → USER PATCHES**, turn **K1** to auto-load a populated slot, or turn **K2** to recall immediately.
- **Import** accepts native `.vcp` records and native Contagion patch SysEx messages. **Export** saves a `.vcp` copy on your computer.

The editor runs offline with the factory data embedded. 1.2 firmware supports USER 1–32. Firmware 1.1 supports USER 1–16. The audition keyboard sends MIDI notes. Listen through the FM‑1 audio output. The native SysEx protocol transfers stored USER slots and does not read the active edit buffer.

After building Contagion, run `node web-editor/generate.cjs` from the repository root to regenerate the standalone HTML.
