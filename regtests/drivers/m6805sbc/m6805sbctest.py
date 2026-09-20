# license:BSD-3-Clause
# copyright-holders:Richard Thomson

import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


THIS_DIR = Path(__file__).resolve().parent
MAME_ROOT = THIS_DIR.parents[2]
SYSTEM_NAME = "m6805sbc"


def parse_args():
	parser = argparse.ArgumentParser(
		description="Run integration tests for the MC146805E2 SBC driver.")
	parser.add_argument(
		"--executable",
		help="Path to a MAME executable, or a name to find on PATH.")
	parser.add_argument(
		"--timeout",
		type=int,
		default=60,
		help="Maximum seconds to allow each MAME run.")
	return parser.parse_args()


def resolve_executable(executable):
	if executable:
		path = Path(executable)
		if path.is_file():
			return path.resolve()
		found = shutil.which(executable)
		if found:
			return Path(found).resolve()
		raise FileNotFoundError("MAME executable not found: {0}".format(executable))

	for candidate in ("mame", "mame.exe", "m6805sbc", "m6805sbc.exe"):
		path = MAME_ROOT / candidate
		if path.is_file():
			return path.resolve()
		found = shutil.which(candidate)
		if found:
			return Path(found).resolve()

	raise FileNotFoundError(
		"No MAME executable found. Use --executable to specify one.")


def write_test_rom(rom_root):
	rom_dir = rom_root / SYSTEM_NAME
	rom_dir.mkdir(parents=True, exist_ok=True)

	rom = bytearray(4096)
	rom[0] = 0x20
	rom[1] = 0xfe
	rom[2] = 0xa5
	rom[0x0ffe] = 0x10
	rom[0x0fff] = 0x00

	rom_path = rom_dir / "{0}.bin".format(SYSTEM_NAME)
	rom_path.write_bytes(rom)
	return rom_path


def render_lua_probe(probe_path, result_path, output_path):
	source = probe_path.read_text(encoding="utf-8")
	source = source.replace("@RESULT_PATH@", result_path.as_posix())
	output_path.write_text(source, encoding="utf-8")


def run_probe(executable, probe_name, probe_path, work_root, timeout):
	rom_root = work_root / "roms"
	write_test_rom(rom_root)

	result_path = work_root / "{0}.result".format(probe_name)
	script_path = work_root / "{0}.lua".format(probe_name)
	render_lua_probe(probe_path, result_path, script_path)

	for directory in ("cfg", "comments", "input", "nvram", "snap", "state"):
		(work_root / directory).mkdir(parents=True, exist_ok=True)

	command = [
		str(executable),
		"-noreadconfig",
		"-rompath", str(rom_root),
		"-cfg_directory", str(work_root / "cfg"),
		"-comment_directory", str(work_root / "comments"),
		"-input_directory", str(work_root / "input"),
		"-nvram_directory", str(work_root / "nvram"),
		"-snapshot_directory", str(work_root / "snap"),
		"-state_directory", str(work_root / "state"),
		"-autoboot_script", str(script_path),
		"-autoboot_delay", "0",
		"-video", "none",
		"-sound", "none",
		"-nothrottle",
		"-skip_gameinfo",
		"-seconds_to_run", "2",
		SYSTEM_NAME,
	]

	process = subprocess.run(
		command,
		cwd=str(MAME_ROOT),
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		text=True,
		timeout=timeout)

	if process.returncode != 0:
		print("{0}: MAME exited with {1}".format(probe_name, process.returncode))
		if process.stdout:
			print(process.stdout)
		if process.stderr:
			print(process.stderr, file=sys.stderr)
		return False

	if not result_path.is_file():
		print("{0}: missing result file {1}".format(probe_name, result_path))
		if process.stdout:
			print(process.stdout)
		if process.stderr:
			print(process.stderr, file=sys.stderr)
		return False

	result = result_path.read_text(encoding="utf-8").splitlines()
	if not result or result[0] != "PASS":
		print("{0}: failed".format(probe_name))
		for line in result:
			print("  {0}".format(line))
		return False

	print("{0}: PASS".format(probe_name))
	return True


def main():
	args = parse_args()

	try:
		executable = resolve_executable(args.executable)
	except FileNotFoundError as err:
		print(err, file=sys.stderr)
		return 2

	probes = [
		("memory_map", THIS_DIR / "lua" / "memory_map.lua"),
	]

	success = True
	with tempfile.TemporaryDirectory(prefix="m6805sbc-") as temp_dir:
		work_root = Path(temp_dir)
		for probe_name, probe_path in probes:
			if not probe_path.is_file():
				print("{0}: missing Lua probe {1}".format(probe_name, probe_path))
				success = False
				continue
			if not run_probe(executable, probe_name, probe_path, work_root, args.timeout):
				success = False

	return 0 if success else 1


if __name__ == "__main__":
	sys.exit(main())
